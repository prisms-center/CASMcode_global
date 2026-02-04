#include <pybind11/eigen.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <fstream>
#include <iostream>

// nlohmann::json binding
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "pybind11_json/pybind11_json.hpp"

// CASM
#include "casm/global/definitions.hh"
#include "casm/global/threads.hh"
#include "casm/global/version.hh"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

/// CASM - Python binding code
namespace CASMpy {

using namespace CASM;

double default_tol() { return TOL; }

}  // namespace CASMpy

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_casmglobal, m) {
  using namespace CASMpy;

  m.doc() = R"pbdoc(
        libcasm.casmglobal
        ------------------

        The libcasm.casmglobal module has CASM global constants and definitions.

        )pbdoc";

  // Default tolerance
  m.attr("TOL") = TOL;

  // Boltzmann Constant
  m.attr("KB") = KB;

  // Planck's Constant
  m.attr("PLANCK") = PLANCK;

  m.def("libcasm_global_version", &libcasm_global_version, R"pbdoc(
      The -lcasm_global version.
      )pbdoc");

  // Thread configuration bindings
  m.def("get_max_threads", &max_threads, R"pbdoc(
      This is equivalent to :func:`max_threads`.
      )pbdoc");
  m.def("max_threads", &max_threads, R"pbdoc(
      A global configuration variable indicating the maximum number of
      threads to use in multithreaded CASM functions.

      By default, this is set to the number of hardware threads.
      )pbdoc");
  m.def("set_max_threads", &set_max_threads, R"pbdoc(
      Set the global configuration variable indicating the maximum number of
      threads to use in multithreaded CASM functions.

      Parameters
      ----------
      n_threads : int
          The maximum number of threads to use.
      )pbdoc",
        py::arg("n_threads"));
  m.def("reset_max_threads", &reset_max_threads, R"pbdoc(
      Reset the maximum number of threads to hardware concurrency.
      )pbdoc");

  // Stop-request bindings
  m.def("stop_requested", &stop_requested, R"pbdoc(
      A flag indicating whether a global stop has been requested (e.g. via
      :func:`request_stop`).
      )pbdoc");
  m.def("request_stop", &request_stop, R"pbdoc(
      Request a global stop; sets the :func:`stop_requested` flag.
      )pbdoc");
  m.def("reset_stop_requested", &reset_stop_requested, R"pbdoc(
      Reset the :func:`stop_requested` flag to False.
      )pbdoc");

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
