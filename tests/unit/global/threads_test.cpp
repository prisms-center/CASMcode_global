#include "casm/global/threads.hh"

#include <atomic>
#include <chrono>
#include <numeric>
#include <optional>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

using namespace CASM;

TEST(ThreadsTest, ThreadedRunSerial) {
  // Save/restore global setting
  Index orig = get_max_threads();
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
  Index orig = get_max_threads();
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
  Index orig = get_max_threads();
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
  Index orig = get_max_threads();
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
  threaded_pipeline(producer, worker, merger, std::optional<Index>(16),
                    std::optional<Index>(16));

  // expected sum = sum_{i=0..N-1} (i+1) = N*(N+1)/2
  long long expected =
      static_cast<long long>(N) * (static_cast<long long>(N) + 1) / 2;
  EXPECT_EQ(sum.load(), expected);

  set_max_threads(orig);
}

// New tests to verify request_stop() and stop_requested() behavior
TEST(ThreadsTest, RequestStopThreadedRun) {
  Index orig = get_max_threads();
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
  Index orig = get_max_threads();
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

  threaded_pipeline(producer, worker, merger, std::optional<Index>(64),
                    std::optional<Index>(64));

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
