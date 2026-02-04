#ifndef CASM_global_threads
#define CASM_global_threads

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <exception>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "casm/global/definitions.hh"
#include "casm/global/eigen.hh"

namespace CASM {

/// \brief A global configuration variable indicating the maximum number of
///     threads to use in CASM.
Index max_threads();

/// \brief Reset the maximum number of threads to the hardware concurrency
void reset_max_threads();

/// \brief Set the maximum number of threads to use
void set_max_threads(Index n_threads);

/// \brief Reset the stop requested flag to false
void reset_stop_requested();

/// \brief Set the stop requested flag to true
void request_stop();

//// \brief Check whether a stop has been requested or SIGINT received
bool stop_requested();

/// \brief RAII helper to temporarily set Eigen's thread count
class EigenThreadLimiter {
 public:
  // Sets threads to 'n' and stores the previous value
  // A value of 0 lets Eigen decide the thread count (usually hardware
  // concurrency)
  explicit EigenThreadLimiter(int n) {
    m_previous = Eigen::nbThreads();
    Eigen::setNbThreads(n);
  }

  // Automatically restores previous thread count on destruction
  ~EigenThreadLimiter() { Eigen::setNbThreads(m_previous); }

  // Delete copy/assignment to prevent accidental resource leaks
  EigenThreadLimiter(const EigenThreadLimiter &) = delete;
  EigenThreadLimiter &operator=(const EigenThreadLimiter &) = delete;

 private:
  int m_previous;
};

/// \brief Run a range-based worker either serially or in parallel
///
/// Configuration:
///  - Uses max_threads() to determine number of threads to use.
///  - If max_threads() is 1, runs the worker serially in the calling
///    thread.
///  - If max_threads() > 1, divides the range [0, n) into approximately
///    equal sub-ranges and runs the worker in multiple threads.
///  - If stop_requested() returns true before starting a worker thread, that
///    thread will not be started; if stop_requested() returns true within a
///    worker thread, that thread should attempt to stop as soon as possible.
///
/// \param n The total range size
/// \param worker The range-based worker function
///
template <typename RangeWorker>
static void threaded_run(Index n, RangeWorker &&worker) {
  Eigen::initParallel();  // Just in case
  EigenThreadLimiter(1);  // Prevent Eigen from using extra threads

  if (n <= 0) return;

  Index n_threads = std::min<Index>(n, std::max<Index>(1, max_threads()));

  // Fast serial path
  if (n_threads == 1) {
    // If a stop has been requested, don't start the serial worker.
    if (stop_requested()) return;
    worker(0, n, 0);
    return;
  }

  Index block = n / n_threads;
  Index rem = n % n_threads;

  std::vector<std::thread> threads;
  threads.reserve(static_cast<size_t>(n_threads));

  std::exception_ptr first_exception = nullptr;
  std::mutex exc_mutex;

  Index start = 0;
  for (Index t = 0; t < n_threads; ++t) {
    Index extra = (t < rem) ? 1 : 0;
    Index end = start + block + extra;
    threads.emplace_back(
        [start, end, &worker, t, &first_exception, &exc_mutex]() {
          try {
            // Check for an externally-requested stop before doing work.
            if (stop_requested()) return;
            worker(start, end, t);
          } catch (...) {
            std::lock_guard<std::mutex> l(exc_mutex);
            if (!first_exception) first_exception = std::current_exception();
          }
        });
    start = end;
  }

  for (auto &th : threads) {
    if (th.joinable()) th.join();
  }

  if (first_exception) std::rethrow_exception(first_exception);
}

/// \brief Producer-consumer pipeline helper
///
/// Implements a standard pattern where:
///  - a single producer repeatedly generates tasks (returning
///    std::optional<Task>),
///  - N-1 worker threads consume tasks and produce results,
///  - one merger thread consumes results and merges them via the provided
///  merger.
///
/// Requirements:
///  - Producer must be callable with signature: std::optional<Task> producer();
///    returning std::nullopt to indicate no more tasks.
///  - Worker must be callable with signature: Result worker(Task task, Index
///  worker_id);
///  - Merger must be callable with signature: void merger(Result result);
///
/// Behavior:
///  - Uses max_threads() to determine total threads; if that is 1 the
///    pipeline runs serially in the calling thread.
///  - Exceptions thrown by any thread are captured and the first is
///    rethrown on return after threads are joined.
///  - If stop_requested() returns true, all threads will attempt to stop as
///    soon as possible.
///
/// \param producer The task producer function, which returns
/// std::optional<Task>.
/// \param worker The worker function, which is called in multiple threads.
/// \param merger The result merger function, which acts on each produced
/// result.
/// \param task_queue_max_size Optional upper bound on how many tasks the
///        controller will buffer; `std::nullopt` (default) means unbounded;
///        a present positive value imposes a blocking bound.
/// \param result_queue_max_size Optional upper bound on how many results the
///        workers may buffer; `std::nullopt` (default) means unbounded; a
///        present positive value imposes a blocking bound.
///
template <typename TaskProducer, typename Worker, typename Merger>
static void threaded_pipeline(
    TaskProducer &&producer, Worker &&worker, Merger &&merger,
    std::optional<Index> task_queue_max_size = std::nullopt,
    std::optional<Index> result_queue_max_size = std::nullopt) {
  Eigen::initParallel();  // Just in case
  EigenThreadLimiter(1);  // Prevent Eigen from using extra threads

  using TaskOpt = std::decay_t<decltype(producer())>;
  // Expect TaskOpt to be std::optional<Task>
  using Task = typename TaskOpt::value_type;
  using Result = std::decay_t<decltype(worker(std::declval<Task>(), 0))>;

  Index total_threads = std::max<Index>(1, max_threads());
  // Serial fallback: produce, process, merge inline
  if (total_threads == 1) {
    while (true) {
      // Respect external stop requests in the serial path.
      if (stop_requested()) break;
      TaskOpt task_opt = producer();
      if (!task_opt) break;
      Result res = worker(std::move(*task_opt), 0);
      merger(std::move(res));
    }
    return;
  }

  const Index n_workers =
      std::max<Index>(1, total_threads - 1);  // worker threads

  std::queue<Task> task_queue;
  std::queue<Result> result_queue;
  std::mutex task_mutex;
  std::condition_variable task_cv;
  std::mutex result_mutex;
  std::condition_variable result_cv;

  std::atomic<bool> producing(true);
  std::atomic<Index> workers_done(0);
  std::exception_ptr first_exception = nullptr;
  std::mutex exc_mutex;
  std::atomic<bool> stop_all(false);

  // Controller thread (producer + merger combined)
  // Produces initial tasks up to n_workers, then repeatedly waits for a result,
  // merges it, and produces the next task (if any).
  std::thread controller_th([&]() {
    try {
      // Initial fill: submit up to configured capacity (or n_workers if
      // unbounded).
      Index initial_fill = (task_queue_max_size && *task_queue_max_size > 0
                                ? *task_queue_max_size
                                : n_workers);
      for (Index i = 0; i < initial_fill && !stop_all && !stop_requested();
           ++i) {
        TaskOpt opt = producer();
        if (!opt) {
          producing = false;
          break;
        }
        // push to task_queue, blocking if bounded and full
        {
          std::unique_lock<std::mutex> lk(task_mutex);
          if (task_queue_max_size && *task_queue_max_size > 0) {
            task_cv.wait(lk, [&]() {
              return stop_all || task_queue.size() <
                                     static_cast<size_t>(*task_queue_max_size);
            });
            if (stop_all) break;
          }
          task_queue.push(std::move(*opt));
        }
        task_cv.notify_one();
      }

      // Replenish-per-result: wait for results, then produce next task,
      // then merge the received result.
      while (!stop_all && !stop_requested()) {
        Result res;
        {
          std::unique_lock<std::mutex> lk(result_mutex);
          result_cv.wait(lk, [&]() {
            return stop_all || !result_queue.empty() ||
                   (workers_done.load() == static_cast<Index>(n_workers) &&
                    !producing);
          });

          if (stop_all) break;

          // If a global stop was requested, ensure all threads are told to
          // stop and wake any waiters.
          if (stop_requested()) {
            stop_all = true;
            task_cv.notify_all();
            result_cv.notify_all();
            break;
          }

          if (result_queue.empty()) {
            // no results; if all workers done and producer finished, we're done
            if (workers_done.load() == static_cast<Index>(n_workers) &&
                !producing)
              break;
            continue;
          }

          res = std::move(result_queue.front());
          result_queue.pop();
        }

        // notify one blocked worker that space in result_queue may be available

        // Produce the next task (if any) before merging the result. Block if
        // the task queue is bounded and full.
        if (!stop_all && !stop_requested()) {
          TaskOpt opt = producer();
          if (!opt) {
            producing = false;
            // notify workers that no more tasks will be produced
            task_cv.notify_all();
            // continue to drain remaining results
            result_cv.notify_all();
            // merge the result even if no new task was produced
            merger(std::move(res));
            continue;
          }
          {
            std::unique_lock<std::mutex> lk(task_mutex);
            if (task_queue_max_size && *task_queue_max_size > 0) {
              task_cv.wait(lk, [&]() {
                return stop_all ||
                       task_queue.size() <
                           static_cast<size_t>(*task_queue_max_size);
              });
              if (stop_all) break;
            }
            task_queue.push(std::move(*opt));
          }
          task_cv.notify_one();
        }

        // Merge the result outside the lock (after producing next task)
        merger(std::move(res));
      }

      // Drain any remaining results
      while (!stop_all && !stop_requested()) {
        Result res;
        {
          std::lock_guard<std::mutex> lk(result_mutex);
          if (result_queue.empty()) break;
          res = std::move(result_queue.front());
          result_queue.pop();
        }

        // notify one blocked worker that space in result_queue may be available
        result_cv.notify_one();

        merger(std::move(res));
      }
    } catch (...) {
      std::lock_guard<std::mutex> l(exc_mutex);
      if (!first_exception) first_exception = std::current_exception();
      stop_all = true;
      task_cv.notify_all();
      result_cv.notify_all();
    }
    // ensure workers know producing has stopped
    producing = false;
    task_cv.notify_all();
    result_cv.notify_all();
  });

  // Worker threads
  std::vector<std::thread> workers;
  workers.reserve(static_cast<size_t>(n_workers));
  for (Index w = 0; w < n_workers; ++w) {
    workers.emplace_back([&, w]() {
      try {
        while (!stop_all && !stop_requested()) {
          Task task;
          {
            std::unique_lock<std::mutex> lk(task_mutex);
            task_cv.wait(lk, [&]() {
              return stop_all || !task_queue.empty() || !producing;
            });
            if (stop_all) break;
            if (stop_requested()) {
              stop_all = true;
              task_cv.notify_all();
              result_cv.notify_all();
              break;
            }
            if (task_queue.empty()) {
              // no tasks and producer finished
              if (!producing) break;
              continue;
            }
            task = std::move(task_queue.front());
            task_queue.pop();
            // notify potential producers waiting to push into task_queue
            lk.unlock();
            task_cv.notify_one();
            // re-lock scope ended; continue processing with task
            // Note: task variable holds the popped task
            {
              // nothing here; keep outer logic consistent
            }
          }

          // Check again for global stop before doing per-task work.
          if (stop_requested()) {
            stop_all = true;
            task_cv.notify_all();
            result_cv.notify_all();
            break;
          }
          Result res = worker(std::move(task), w);

          {
            std::unique_lock<std::mutex> lk(result_mutex);
            if (result_queue_max_size && *result_queue_max_size > 0) {
              result_cv.wait(lk, [&]() {
                return stop_all ||
                       result_queue.size() <
                           static_cast<size_t>(*result_queue_max_size);
              });
              if (stop_all) break;
            }
            result_queue.push(std::move(res));
          }
          // notify controller that a result is available
          result_cv.notify_one();
        }
      } catch (...) {
        std::lock_guard<std::mutex> l(exc_mutex);
        if (!first_exception) first_exception = std::current_exception();
        stop_all = true;
        task_cv.notify_all();
        result_cv.notify_all();
      }
      ++workers_done;
      result_cv.notify_all();
    });
  }

  // Join all threads
  controller_th.join();
  for (auto &th : workers)
    if (th.joinable()) th.join();

  if (first_exception) std::rethrow_exception(first_exception);
}

}  // namespace CASM

#endif
