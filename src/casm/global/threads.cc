#include "casm/global/threads.hh"

namespace CASM {
/// \brief A global configuration variable indicating the maximum number of
///     threads to use in CASM.
///
/// - Defaults to the number of hardware threads.
/// - Can be set via set_max_threads().
/// - Can be reset to the hardware concurrency via reset_max_threads().
/// - Is respected by threaded_run() and threaded_pipeline().
Index &get_max_threads() {
  static Index max_threads = std::thread::hardware_concurrency();
  if (max_threads == 0) {
    max_threads = 1;
  }
  return max_threads;
}

/// \brief Reset the maximum number of threads to the hardware concurrency
void reset_max_threads() {
  Index max_threads = std::thread::hardware_concurrency();
  if (max_threads <= 0) {
    max_threads = 1;
  }
  set_max_threads(max_threads);
}

/// \brief Set the maximum number of threads to use
void set_max_threads(Index n_threads) {
  Index &max_threads = get_max_threads();
  if (n_threads <= 0) {
    n_threads = 1;
  }
  max_threads = n_threads;
}

/// \brief An atomic flag indicating whether a stop has been requested, for
///     example by ctrl-c / SIGINT.
std::atomic<bool> &get_stop_requested() {
  // Global or static so the signal handler can see it
  static std::atomic<bool> global_stop_requested{false};
  return global_stop_requested;
}

/// \brief Reset the stop requested flag to false
void reset_stop_requested() {
  static std::atomic<bool> &stop_requested = get_stop_requested();
  stop_requested.store(false, std::memory_order_relaxed);
  sigint_requested = 0;
}

/// \brief Set the stop requested flag to true
void request_stop() {
  static std::atomic<bool> &stop_requested = get_stop_requested();
  stop_requested.store(true, std::memory_order_relaxed);
  // Mirror into signal-safe flag for immediate visibility to code
  // that polls only the sig_atomic_t.
  sigint_requested = 1;
}

/// \brief Check whether a stop has been requested
bool stop_requested() {
  static std::atomic<bool> &stop_requested = get_stop_requested();
  // Consider either the atomic or the signal-safe flag.
  return stop_requested.load(std::memory_order_relaxed) ||
         sigint_requested != 0;
}

}  // namespace CASM