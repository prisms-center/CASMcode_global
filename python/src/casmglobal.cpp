#include <pybind11/eigen.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <fstream>

// nlohmann::json binding
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "pybind11_json/pybind11_json.hpp"

// CASM
#include "casm/global/definitions.hh"

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

  // Test function
  m.def(
      "add", [](double a, double b) { return a + b; }, py::arg("a"),
      py::arg("b"),
      R"pbdoc(
      Add two numbers

      Parameters
      ----------
      a : float
          First number.
      a : float
          Second number.


      Returns
      -------
      sum: float
          The result of a + b.
      )pbdoc");

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
