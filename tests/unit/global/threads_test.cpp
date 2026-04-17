#include "casm/global/threads.hh"

#include <atomic>
#include <chrono>
#include <numeric>
#include <optional>
#include <set>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

using namespace CASM;

TEST(ThreadsTest, ThreadedRunSerial) {
  // Save/restore global setting
  Index orig = max_threads();
  set_max_threads(1);

  const Index n = 10;
  std::vector<std::atomic<int>> seen(n);
  for (Index i = 0; i < n; ++i) seen[static_cast<size_t>(i)].store(-1);

  threaded_run(n, [&](Index start, Index end, Index tid) {
    for (Index i = start; i < end; ++i) {
      seen[static_cast<size_t>(i)].store(static_cast<int>(tid));
    }
  });

  // In serial mode the single worker is given id 0 and should have processed
  // the entire range.
  for (Index i = 0; i < n; ++i) {
    EXPECT_EQ(seen[static_cast<size_t>(i)].load(), 0);
  }

  set_max_threads(orig);
}

TEST(ThreadsTest, ThreadedRunParallel) {
  Index orig = max_threads();
  // Request multiple threads to exercise the parallel path
  set_max_threads(4);

  const Index n = 37;  // non-multiple to exercise remainder logic
  std::vector<std::atomic<int>> seen(n);
  for (Index i = 0; i < n; ++i) seen[static_cast<size_t>(i)].store(-1);

  threaded_run(n, [&](Index start, Index end, Index tid) {
    for (Index i = start; i < end; ++i) {
      // mark which thread handled this index
      seen[static_cast<size_t>(i)].store(static_cast<int>(tid));
    }
  });

  // All indices must have been processed and set to a non-negative thread id
  for (Index i = 0; i < n; ++i) {
    int v = seen[static_cast<size_t>(i)].load();
    EXPECT_GE(v, 0);
  }

  set_max_threads(orig);
}

TEST(ThreadsTest, ThreadedPipelineSerial) {
  Index orig = max_threads();
  set_max_threads(1);

  const Index N = 100;
  Index produced = 0;

  auto producer = [&]() -> std::optional<Index> {
    if (produced >= N) return std::nullopt;
    return std::optional<Index>(produced++);
  };

  auto worker = [&](Index task, Index /*worker_id*/) {
    // simple transform: double the value
    return task * 2;
  };

  std::atomic<long long> sum{0};
  auto merger = [&](long long res) {
    sum.fetch_add(res, std::memory_order_relaxed);
  };

  threaded_pipeline(producer, worker, merger);

  // expected sum = 2 * sum_{i=0..N-1} i = 2 * N*(N-1)/2 = N*(N-1)
  long long expected =
      static_cast<long long>(N) * static_cast<long long>(N - 1);
  EXPECT_EQ(sum.load(), expected);

  set_max_threads(orig);
}

TEST(ThreadsTest, ThreadedPipelineParallelBounded) {
  Index orig = max_threads();
  set_max_threads(4);

  const Index N = 1000;
  std::atomic<Index> produced{0};

  auto producer = [&]() -> std::optional<Index> {
    Index cur = produced.fetch_add(1);
    if (cur >= N) return std::nullopt;
    return std::optional<Index>(cur);
  };

  auto worker = [&](Index task, Index /*worker_id*/) {
    return task + 1;  // return simple transformed value
  };

  std::atomic<long long> sum{0};
  auto merger = [&](long long res) {
    sum.fetch_add(res, std::memory_order_relaxed);
  };

  // Use bounded queues to exercise blocking behavior
  threaded_pipeline(producer, worker, merger, 16);

  // expected sum = sum_{i=0..N-1} (i+1) = N*(N+1)/2
  long long expected =
      static_cast<long long>(N) * (static_cast<long long>(N) + 1) / 2;
  EXPECT_EQ(sum.load(), expected);

  set_max_threads(orig);
}

/// Stress test for the bounded-task / unbounded-result configuration of
/// threaded_pipeline. This matches the configuration used by
/// MakeAllSubgroupsFromGenerators::run() in CASMcode_configuration
/// (task_queue_max_size = max_threads()+10).
///
/// The suspected deadlock (Suspect 1 in thread_locking_notes.md) arises because
/// task_cv is shared between:
///   - workers waiting for items  (!task_queue.empty())
///   - the controller waiting for space  (task_queue.size() < max_size)
/// A task_cv.notify_one() from a worker that just popped a task may wake
/// another idle worker instead of the controller, leaving the controller
/// blocked forever.
///
/// The test hangs (does not assert-fail) on a deadlock, consistent with what is
/// observed in the CASMcode_configuration stress tests.
TEST(ThreadsTest, PipelineBoundedTaskUnboundedResultStress) {
  Index orig = max_threads();
  set_max_threads(8);

  const Index N_tasks = 500;
  const Index N_repeat = 200;

  for (Index rep = 0; rep < N_repeat; ++rep) {
    std::atomic<Index> produced{0};

    auto producer = [&]() -> std::optional<Index> {
      Index cur = produced.fetch_add(1, std::memory_order_relaxed);
      if (cur >= N_tasks) return std::nullopt;
      return cur;
    };

    auto worker = [](Index task, Index /*worker_id*/) -> Index { return task; };

    std::atomic<long long> sum{0};
    auto merger = [&](Index res) {
      sum.fetch_add(static_cast<long long>(res), std::memory_order_relaxed);
    };

    // bounded task queue, unbounded result queue — matches subgroup finder
    threaded_pipeline(producer, worker, merger,
                      /*task_queue_max_size=*/max_threads() + 10);

    long long expected = static_cast<long long>(N_tasks) *
                         (static_cast<long long>(N_tasks) - 1) / 2;
    ASSERT_EQ(sum.load(), expected) << "Wrong sum on repetition " << rep;
  }

  set_max_threads(orig);
}

/// Stress test that adds a shared_mutex contention pattern to the worker,
/// simulating how MakeAllSubgroupsFromGenerators workers access the shared
/// `subgroups` map: many shared_lock reads, rare unique_lock writes.
///
/// If the deadlock is purely in threaded_pipeline mechanics, this test should
/// also hang. If it passes, the bug requires something specific to the
/// MakeAllSubgroupsFromGenerators worker logic (e.g., tree traversal state).
TEST(ThreadsTest, PipelineBoundedTaskSharedMutexStress) {
  Index orig = max_threads();
  set_max_threads(8);

  const Index N_tasks = 500;
  const Index N_repeat = 200;
  // Simulated shared "found set" — workers read-check and occasionally write,
  // mirroring subgroups_count() / subgroups_emplace() in the real code.
  std::shared_mutex found_mutex;
  std::set<Index> found_set;

  for (Index rep = 0; rep < N_repeat; ++rep) {
    found_set.clear();
    std::atomic<Index> produced{0};

    auto producer = [&]() -> std::optional<Index> {
      Index cur = produced.fetch_add(1, std::memory_order_relaxed);
      if (cur >= N_tasks) return std::nullopt;
      return cur;
    };

    // Worker: do CPU work (simulate tree traversal) plus shared_mutex accesses
    // (simulate subgroups_count / subgroups_emplace).
    auto worker = [&](Index task, Index /*worker_id*/) -> Index {
      // Simulate ~10 subgroup-check iterations per task.
      for (Index i = 0; i < 10; ++i) {
        Index key = task * 10 + i;
        // Read check (shared lock) — most iterations.
        {
          std::shared_lock lk(found_mutex);
          if (found_set.count(key)) continue;
        }
        // Write (unique lock) — occasional.
        {
          std::unique_lock lk(found_mutex);
          found_set.insert(key);
        }
      }
      return task;
    };

    std::atomic<long long> sum{0};
    auto merger = [&](Index res) {
      sum.fetch_add(static_cast<long long>(res), std::memory_order_relaxed);
    };

    threaded_pipeline(producer, worker, merger,
                      /*task_queue_max_size=*/max_threads() + 10);

    long long expected = static_cast<long long>(N_tasks) *
                         (static_cast<long long>(N_tasks) - 1) / 2;
    ASSERT_EQ(sum.load(), expected) << "Wrong sum on repetition " << rep;
  }

  set_max_threads(orig);
}

/// Regression test for the task_cv wakeup-stealing deadlock in
/// threaded_pipeline.
///
/// Root cause: task_cv is shared between:
///   - workers waiting for items  (!task_queue.empty())
///   - the controller waiting for space  (task_queue.size() < max_size)
/// When a worker pops a task and calls task_cv.notify_one(), the notification
/// may wake another idle worker (also waiting on task_cv for items) instead of
/// the controller (waiting on task_cv for space). If all workers become idle
/// and the controller is also waiting for space, nobody wakes the controller,
/// and the pipeline deadlocks permanently.
///
/// This test is designed to reproduce that deadlock. With the buggy
/// implementation it should hang (and the per-rep timeout will fire, causing
/// the test to fail). After the fix (splitting task_cv into task_item_cv and
/// task_space_cv) the test should pass reliably.
///
/// Parameters match the real failing scenario in CASMcode_configuration:
///   max_threads=12, n_workers=11, task_queue_max_size=22, n_tasks=100,
///   trivial (instant) workers.
TEST(ThreadsTest, PipelineBoundedDeadlockRegression) {
  Index orig = max_threads();
  set_max_threads(12);

  const Index N_tasks = 100;
  const Index N_repeat = 50;
  const auto PER_REP_TIMEOUT = std::chrono::seconds(5);

  for (Index rep = 0; rep < N_repeat; ++rep) {
    std::atomic<Index> produced{0};

    auto producer = [&]() -> std::optional<Index> {
      Index cur = produced.fetch_add(1, std::memory_order_relaxed);
      if (cur >= N_tasks) return std::nullopt;
      return cur;
    };

    // Workers sleep briefly so the controller can fill the task queue to
    // capacity and block on task_cv waiting for space. Without this delay,
    // workers drain tasks faster than the controller produces them; the queue
    // never fills, the controller never blocks, and the race cannot occur.
    auto worker = [](Index task, Index /*worker_id*/) -> Index {
      std::this_thread::sleep_for(std::chrono::microseconds(200));
      return task;
    };

    std::atomic<long long> sum{0};
    auto merger = [&](Index res) {
      sum.fetch_add(static_cast<long long>(res), std::memory_order_relaxed);
    };

    std::atomic<bool> done{false};
    std::thread pipeline_thread([&]() {
      threaded_pipeline(producer, worker, merger,
                        /*task_queue_max_size=*/max_threads() + 10);
      done.store(true, std::memory_order_release);
    });

    auto deadline = std::chrono::steady_clock::now() + PER_REP_TIMEOUT;
    while (!done.load(std::memory_order_acquire) &&
           std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!done.load(std::memory_order_acquire)) {
      // Pipeline deadlocked — detach to avoid blocking join, then fail.
      pipeline_thread.detach();
      set_max_threads(orig);
      ASSERT_TRUE(false) << "threaded_pipeline deadlocked on rep " << rep
                         << " (task_cv wakeup-stealing bug)";
    }

    pipeline_thread.join();

    long long expected = static_cast<long long>(N_tasks) *
                         (static_cast<long long>(N_tasks) - 1) / 2;
    ASSERT_EQ(sum.load(), expected) << "Wrong sum on rep " << rep;
  }

  set_max_threads(orig);
}

// New tests to verify request_stop() and stop_requested() behavior
TEST(ThreadsTest, RequestStopThreadedRun) {
  Index orig = max_threads();
  set_max_threads(4);
  reset_stop_requested();

  const Index n = 10000;  // sufficiently large work
  std::atomic<Index> processed{0};

  // start a thread that requests stop shortly after work begins
  std::thread stopper([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    request_stop();
  });

  threaded_run(n, [&](Index start, Index end, Index /*tid*/) {
    for (Index i = start; i < end; ++i) {
      if (stop_requested()) break;
      processed.fetch_add(1, std::memory_order_relaxed);
      // small sleep to ensure work is not instantaneous
      std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
  });

  stopper.join();

  // The stop flag should have been set and some but not all work performed
  EXPECT_TRUE(stop_requested());
  EXPECT_GT(processed.load(), 0);
  EXPECT_LT(processed.load(), n);

  reset_stop_requested();
  set_max_threads(orig);
}

TEST(ThreadsTest, RequestStopThreadedPipeline) {
  Index orig = max_threads();
  set_max_threads(4);
  reset_stop_requested();

  const Index N = 2000;
  std::atomic<Index> produced{0};

  auto producer = [&]() -> std::optional<Index> {
    Index cur = produced.fetch_add(1);
    if (cur >= N) return std::nullopt;
    return std::optional<Index>(cur);
  };

  auto worker = [&](Index task, Index /*worker_id*/) {
    // simulate some work per task
    std::this_thread::sleep_for(std::chrono::microseconds(50));
    return task + 1;
  };

  std::atomic<long long> sum{0};
  auto merger = [&](long long res) {
    sum.fetch_add(res, std::memory_order_relaxed);
  };

  // Start a stopper that requests stop shortly after the pipeline starts
  std::thread stopper([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    request_stop();
  });

  threaded_pipeline(producer, worker, merger, 64);

  stopper.join();

  // Stop should have been requested and only partial work should be completed
  EXPECT_TRUE(stop_requested());
  long long completed = sum.load();
  EXPECT_GT(completed, 0);
  // full expected sum would be N*(N+1)/2; after stop we should have less
  long long full_expected =
      static_cast<long long>(N) * (static_cast<long long>(N) + 1) / 2;
  EXPECT_LT(completed, full_expected);

  reset_stop_requested();
  set_max_threads(orig);
}
