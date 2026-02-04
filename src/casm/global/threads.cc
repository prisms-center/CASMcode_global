#include "casm/global/threads.hh"

#include <iostream>

namespace CASM {
/// \brief A global configuration variable indicating the maximum number of
///     threads to use in CASM.
///
/// - Defaults to the number of hardware threads.
/// - Can be set via set_max_threads().
/// - Can be reset to the hardware concurrency via reset_max_threads().
/// - Is respected by threaded_run() and threaded_pipeline().
Index &_get_max_threads() {
  static Index max_threads = std::thread::hardware_concurrency();
  if (max_threads == 0) {
    max_threads = 1;
  }
  return max_threads;
}

/// \brief A global configuration variable indicating the maximum number of
///     threads to use in CASM.
///
/// - Defaults to the number of hardware threads.
/// - Can be set via set_max_threads().
/// - Can be reset to the hardware concurrency via reset_max_threads().
/// - Is respected by threaded_run() and threaded_pipeline().
Index max_threads() { return _get_max_threads(); }

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
  if (n_threads <= 0) {
    n_threads = 1;
  }
  _get_max_threads() = n_threads;
}

// volatile std::sig_atomic_t sigint_requested = 0;

/// \brief An atomic flag indicating whether a stop has been requested
///
/// Notes:
/// - This can be set by a signal handler waiting for SIGINT (Ctrl-C) or
///   other logic.
/// - On platforms where std::atomic<bool> is not always lock-free,
///   a volatile sig_atomic_t is used instead for async-signal-safety.
std::atomic<bool> &_get_stop_requested() {
  static std::atomic<bool> global_stop_requested{false};
  return global_stop_requested;
}

/// \brief A reference to the global sigint_requested flag
///
/// Notes:
/// - On platforms where std::atomic<bool> is not always lock-free,
///   a volatile sig_atomic_t is used instead for async-signal-safety.
volatile std::sig_atomic_t &_get_sigint_requested() {
  static volatile std::sig_atomic_t global_sigint_requested = 0;
  return global_sigint_requested;
}

/// \brief Reset the stop requested flag to false
void reset_stop_requested() {
  if constexpr (std::atomic<bool>::is_always_lock_free) {
    _get_stop_requested().store(false, std::memory_order_relaxed);
  } else {
    _get_sigint_requested() = 0;
  }
}

/// \brief Set the stop requested flag to true
void request_stop() {
  if constexpr (std::atomic<bool>::is_always_lock_free) {
    _get_stop_requested().store(true, std::memory_order_relaxed);
  } else {
    _get_sigint_requested() = 1;
  }
}

/// \brief Check whether a stop has been requested or SIGINT received
bool stop_requested() {
  if constexpr (std::atomic<bool>::is_always_lock_free) {
    return _get_stop_requested().load(std::memory_order_relaxed);
  } else {
    return _get_sigint_requested() != 0;
  }
}

}  // namespace CASM