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
  threads.reserve(static_cast<std::size_t>(n_threads));

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
///  - a single producer thread repeatedly generates tasks (returning
///    std::optional<Task>) and merges results, while
///  - N-1 worker threads consume tasks and return results,
///
/// Requirements:
///  - Producer function must be a callable with signature:
///    ``std::optional<Task> producer();``, returning ``std::nullopt`` to
///    indicate no more tasks.
///  - Worker function must be a callable with signature
///    ``Result worker(Task task, Index worker_id);``
///  - Merging function must be callable with signature
///    ``void merger(Result result);``
///
/// Behavior:
///  - Uses max_threads() to determine total threads; if that is 1 the
///    pipeline runs serially in the calling thread.
///  - Exceptions thrown by any thread are captured and the first is
///    rethrown on return after threads are joined. If an exception is
///    caught, request_stop() will be called, setting the stop_requested() flag
///    to true.
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
template <typename TaskProducer, typename Worker, typename Merger>
static void threaded_pipeline(
    TaskProducer &&producer, Worker &&worker, Merger &&merger,
    std::optional<Index> task_queue_max_size = std::nullopt) {
  using TaskOpt = std::decay_t<decltype(producer())>;
  // Expect TaskOpt to be std::optional<Task>
  using Task = typename TaskOpt::value_type;
  using Result = std::decay_t<decltype(worker(std::declval<Task>(), 0))>;

  Index total_threads = std::max<Index>(1, max_threads());

  // -- Serial pipeline ---------------------------------------------------
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
  // -- End serial pipeline -----------------------------------------------

  const Index n_workers =
      std::max<Index>(1, total_threads - 1);  // worker threads

  if (task_queue_max_size.has_value() &&
      task_queue_max_size.value() < n_workers) {
    task_queue_max_size = n_workers;
  }
  if (!task_queue_max_size.has_value()) {
    task_queue_max_size = n_workers;
  }

  // -- Pipeline synchronization variables --

  std::mutex pipeline_mutex;

  // Queue of tasks
  // - Fill with up to task_queue_max_size.value() tasks
  std::queue<Task> task_queue;

  // Is producer still producing?
  std::atomic<bool> producing(true);

  // Condition variable to notify workers
  std::condition_variable worker_cv;

  // Queue of results
  std::queue<Result> result_queue;

  // Condition variable to notify producer
  std::condition_variable producer_cv;

  // How many workers have seen !producing (which means they must have finished)
  std::atomic<Index> workers_done(0);

  // Exception handling
  std::exception_ptr first_exception = nullptr;
  std::mutex exc_mutex;

  // -- Fucntions -----------------------------------------

  // Producer: initial fill of task queue
  auto producer_initial_fill = [&]() {
    // Initial fill until:
    // Either 1) *task_queue_max_size tasks added, or 2) until done producing
    Index initial_fill = *task_queue_max_size;
    for (Index i = 0; i < initial_fill; ++i) {
      if (stop_requested()) {
        {
          std::lock_guard<std::mutex> lk(pipeline_mutex);
          producing = false;
          task_queue = std::queue<Task>{};
        }
        worker_cv.notify_all();
        break;
      }

      TaskOpt task_or_null = producer();

      // if done producing:
      if (!task_or_null) {
        {
          std::lock_guard<std::mutex> lk(pipeline_mutex);
          producing = false;
        }
        worker_cv.notify_all();
        break;
      }

      // if new task produced, push to task_queue
      {
        std::lock_guard<std::mutex> lk(pipeline_mutex);
        task_queue.push(std::move(*task_or_null));
      }
      worker_cv.notify_one();
    }
  };

  // Producer:
  // - Wait for a result, merge result, produce task, repeat until no more tasks
  // and no more workers working
  auto producer_loop = [&]() {
    while (true) {
      if (stop_requested()) {
        {
          std::lock_guard<std::mutex> lk(pipeline_mutex);
          producing = false;
          task_queue = std::queue<Task>{};
        }
        worker_cv.notify_all();
        break;
      }

      Result res;

      // Wait for a result or all workers are done
      {
        std::unique_lock<std::mutex> lk(pipeline_mutex);
        producer_cv.wait(lk, [&]() {
          return !result_queue.empty() ||
                 workers_done.load() == static_cast<Index>(n_workers);
        });

        // All workers are done - We are done
        if (result_queue.empty()) {
          return;
        }

        // Get a result from the queue
        res = std::move(result_queue.front());
        result_queue.pop();
      }

      // Merge the result
      if (stop_requested()) {
        continue;
      }
      merger(std::move(res));

      // If still producing:
      if (stop_requested()) {
        continue;
      }
      if (producing) {
        // Produce a task or not
        TaskOpt task_or_null = producer();
        {
          std::lock_guard<std::mutex> lk(pipeline_mutex);
          if (!task_or_null) {
            // if no task, we're done producing
            producing = false;
          } else {
            // if new task produced, push to task_queue
            task_queue.push(std::move(*task_or_null));
          }
        }
      }

      // Either:
      // - 1) a new task has been produced or
      // - 2) we are done producing and waiting for all workers to return
      worker_cv.notify_one();
    }
  };

  auto worker_loop = [&](Index w_id) {
    while (true) {
      // Wait for: 1) a task, or 2) !producing flag
      Task task;
      {
        std::unique_lock<std::mutex> lk(pipeline_mutex);
        worker_cv.wait(lk, [&]() { return !task_queue.empty() || !producing; });

        // If task_queue.empty(), then tasks are no longer being produced
        if (task_queue.empty()) {
          ++workers_done;
          // Notify producer thread that we've finishing.
          producer_cv.notify_one();
          return;
        }

        // Get task
        task = std::move(task_queue.front());
        task_queue.pop();

        // Release pipeline_mutex
      }

      // Do work
      Result res = worker(std::move(task), w_id);

      // Put result in result_queue
      {
        std::lock_guard<std::mutex> lk(pipeline_mutex);
        result_queue.push(std::move(res));
      }

      // Notify producer thread that a result is available
      producer_cv.notify_one();
    }
  };

  // Producer thread: produce tasks and merge results
  // Produces initial tasks up to n_workers, then repeatedly waits for a
  // result, merges it, and produces the next task (if any).
  std::thread controller_th([&]() {
    try {
      producer_initial_fill();
      producer_loop();
    } catch (...) {
      request_stop();
      {
        std::lock_guard<std::mutex> lk(exc_mutex);
        if (!first_exception) first_exception = std::current_exception();
      }
      {
        std::lock_guard<std::mutex> lk(pipeline_mutex);
        producing = false;
        task_queue = std::queue<Task>{};
      }
      worker_cv.notify_all();
    }
  });

  // Worker threads
  std::vector<std::thread> workers;
  workers.reserve(static_cast<std::size_t>(n_workers));
  for (Index w_id = 0; w_id < n_workers; ++w_id) {
    workers.emplace_back([&, w_id]() {
      try {
        worker_loop(w_id);
      } catch (...) {
        request_stop();
        {
          std::lock_guard<std::mutex> lk(exc_mutex);
          if (!first_exception) first_exception = std::current_exception();
        }
        {
          std::lock_guard<std::mutex> lk(pipeline_mutex);
          producing = false;
          task_queue = std::queue<Task>{};
          ++workers_done;
        }
        worker_cv.notify_all();
        producer_cv.notify_one();
      }
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
