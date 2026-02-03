#ifndef CASM_global_pybind11_helpers
#define CASM_global_pybind11_helpers

#include <pybind11/pybind11.h>
#include <unistd.h>

#include "casm/global/threads.hh"

// C++ signal handler (async-signal-safe)
extern "C" inline void libcasm_handle_sigint(int /*sig*/) {
  const char msg[] = "\nReceived Interrupt! Requesting stop...\n\n";
  ssize_t ignored = write(STDOUT_FILENO, msg, sizeof(msg) - 1);
  (void)ignored;
  CASM::sigint_requested = 1;
}

/// CASM - Python binding code
namespace CASMpy {
namespace py = pybind11;
using namespace CASM;

// Reusable helper: run a callable with a temporary SIGINT handler and the
// Python GIL released. Restores the old handler on return or on exception.
// Returns whatever the callable returns (supports references via
// decltype(auto)).

/// \brief Run a callable with a temporary SIGINT handler and the Python GIL
///     released.
///
/// - In order for CASM to handle SIGINT (Ctrl-C) gracefully during long-running
///   operations, this helper sets a temporary signal handler for SIGINT.
/// - Releases the Python GIL while running the callable, in order to allow
///   other Python threads to run and potentially handle SIGINT.
/// - Restores the old handler on return or on exception.
/// - If a SIGINT was received during the callable, requests a stop via
///   CASM::request_stop() after restoring the old handler.
///
/// \param f The callable to run.
/// \returns Whatever the callable returns.
///
template <typename F>
decltype(auto) run_with_sigint_handler(F &&f) {
  auto old_handler = PyOS_setsig(SIGINT, libcasm_handle_sigint);
  try {
    decltype(auto) res = [&]() -> decltype(auto) {
      py::gil_scoped_release release;
      return std::forward<F>(f)();
    }();
    PyOS_setsig(SIGINT, old_handler);
    if (sigint_requested) {
      request_stop();
    }
    return res;
  } catch (...) {
    PyOS_setsig(SIGINT, old_handler);
    if (sigint_requested) {
      request_stop();
    }
    throw;
  }
}
}  // namespace CASMpy

#endif
