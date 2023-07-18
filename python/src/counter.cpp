#include <pybind11/eigen.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <fstream>

// nlohmann::json binding
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "pybind11_json/pybind11_json.hpp"

// CASM
#include "casm/container/Counter.hh"
#include "casm/global/definitions.hh"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

/// CASM - Python binding code
namespace CASMpy {

using namespace CASM;

double default_tol() { return TOL; }

EigenCounter<Eigen::VectorXd> make_float_counter(
    Eigen::VectorXd const &initial, Eigen::VectorXd const &final,
    Eigen::VectorXd const &increment, double tol) {
  return EigenCounter<Eigen::VectorXd>(
      initial, final, increment,
      CASM_TMP::ParenthesesAccess<Eigen::VectorXd, Eigen::VectorXd::Scalar,
                                  Eigen::VectorXd::Index>(),
      CASM_TMP::MuchLessThan<Eigen::VectorXd::Scalar>(tol));
}

}  // namespace CASMpy

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_counter, m) {
  using namespace CASMpy;

  m.doc() = R"pbdoc(
        libcasm.counter._counter
        ------------------------

        CASM counters allow looping over many incrementing variables in one loop.

        )pbdoc";

  py::class_<EigenCounter<Eigen::VectorXi>>(m, "IntCounter", R"pbdoc(
      IntCounter allows looping over many incrementing integer variables in one loop

      An IntCounter is initialized with `initial`, `final`, and `increment` arrays of the same length. The value in each element of `initial` is incremented by the corresponding value in `increment` in a series of loops, where the first element increments in the inner loop and the final element increments in the outer loop. The values in `final` give the (inclusive) final value each element is allowed to take. Increment values may be positive, negative, or mixed, and the corresponding elements of the `initial` and `final` arrays should be set according.

      Example:

      .. code-block:: Python

          counter = IntCounter(
              initial=[0, 0, 0],
              final=[1, 1, 1],
              increment=[1, 1, 1],
          )
          for x in counter:
              print(x)

      Expected output:

      .. code-block:: Python

          array([0, 0, 0])
          array([1, 0, 0])
          array([0, 1, 0])
          array([1, 1, 0])
          array([0, 0, 1])
          array([1, 0, 1])
          array([0, 1, 1])
          array([1, 1, 1])

      )pbdoc")
      .def(py::init<Eigen::VectorXi const &, Eigen::VectorXi const &,
                    Eigen::VectorXi const &>(),
           py::arg("initial"), py::arg("final"), py::arg("increment"), R"pbdoc(
          Constructor

          Parameters
          ----------
          initial : array_like, shape=(n,1)
              Initial value array.
          final : array_like, shape=(n,1)
              Final value array (inclusive).
          increment : array_like, shape=(n,1)
              How much each array element should be incremented. Element values may be positive, negative, or mixed.
          )pbdoc")
      .def(
          "valid", [](EigenCounter<Eigen::VectorXi> &c) { return c.valid(); },
          "Return True if the counter has not yet iterated past the final "
          "value.")
      .def(
          "reset", [](EigenCounter<Eigen::VectorXi> &c) { c.reset(); },
          "Reset counter to the initial value.")
      .def(
          "initial",
          [](EigenCounter<Eigen::VectorXi> &c) { return c.initial(); },
          "Return the initial value.")
      .def(
          "final", [](EigenCounter<Eigen::VectorXi> &c) { return c.final(); },
          "Return the final value.")
      .def(
          "increment",
          [](EigenCounter<Eigen::VectorXi> &c) { return c.increment(); },
          "Return the increment value.")
      .def(
          "next",
          [](EigenCounter<Eigen::VectorXi> &c) {
            ++c;
            return c.current();
          },
          "Increment and return next value.")
      .def(
          "current",
          [](EigenCounter<Eigen::VectorXi> &c) { return c.current(); },
          "Return the current value.")
      .def(
          "size", [](EigenCounter<Eigen::VectorXi> &c) { return c.size(); },
          "Return the size of the array being counted.")
      .def(
          "element",
          [](EigenCounter<Eigen::VectorXi> &c, int i) { return c.current(i); },
          "Return the i-th element of the current array.", py::arg("i"))
      .def(
          "__iter__",
          [](EigenCounter<Eigen::VectorXi> &c) {
            return py::make_iterator(c.begin(), c.end());
          },
          py::keep_alive<
              0, 1>() /* Essential: keep object alive while iterator exists */);

  py::class_<EigenCounter<Eigen::VectorXd>>(m, "FloatCounter", R"pbdoc(
      FloatCounter allows looping over many incrementing float variables in one loop

      A FloatCounter is initialized with `initial`, `final`, and `increment` arrays of the same length. The value in each element of `initial` is incremented by the corresponding value in `increment` in a series of loops, where the first element increments in the inner loop and the final element increments in the outer loop. The values in `final` give the (inclusive) final value each element is allowed to take. Increment values may be positive, negative, or mixed, and the corresponding elements of the `initial` and `final` arrays should be set according.

      Example:

      .. code-block:: Python

          counter = FloatCounter(
              initial=[0, 0, 0],
              final=[0.1, 0.1, 0.1],
              increment=[0.1, 0.1, 0.1],
              tol=1e-5,
          )
          for x in counter:
              print(x)

      Expected output:

      .. code-block:: Python

          array([0.0, 0.0, 0.0])
          array([0.1, 0.0, 0.0])
          array([0.0, 0.1, 0.0])
          array([0.1, 0.1, 0.0])
          array([0.0, 0.0, 0.1])
          array([0.1, 0.0, 0.1])
          array([0.0, 0.1, 0.1])
          array([0.1, 0.1, 0.1])

      )pbdoc")
      .def(py::init(&make_float_counter), py::arg("initial"), py::arg("final"),
           py::arg("increment"), py::arg("tol") = TOL, R"pbdoc(
          Constructor

          Parameters
          ----------
          initial : array_like, shape=(n,1)
              Initial value array.
          final : array_like, shape=(n,1)
              Final value array (inclusive).
          increment : array_like, shape=(n,1)
              How much each array element should be incremented. Element values may be positive, negative, or mixed.
          tol : float = ~libcasm.casmglobal.TOL
              Tolerance for floating point comparisons.
          )pbdoc")
      .def(
          "valid", [](EigenCounter<Eigen::VectorXd> &c) { return c.valid(); },
          "Return True if the counter has not yet iterated past the final "
          "value.")
      .def(
          "reset", [](EigenCounter<Eigen::VectorXd> &c) { c.reset(); },
          "Reset counter to the initial value.")
      .def(
          "initial",
          [](EigenCounter<Eigen::VectorXd> &c) { return c.initial(); },
          "Return the initial value.")
      .def(
          "final", [](EigenCounter<Eigen::VectorXd> &c) { return c.final(); },
          "Return the final value.")
      .def(
          "increment",
          [](EigenCounter<Eigen::VectorXd> &c) { return c.increment(); },
          "Return the increment value.")
      .def(
          "next",
          [](EigenCounter<Eigen::VectorXd> &c) {
            ++c;
            return c.current();
          },
          "Increment and return next value.")
      .def(
          "current",
          [](EigenCounter<Eigen::VectorXd> &c) { return c.current(); },
          "Return the current value.")
      .def(
          "size", [](EigenCounter<Eigen::VectorXd> &c) { return c.size(); },
          "Return the size of the array being counted.")
      .def(
          "element",
          [](EigenCounter<Eigen::VectorXd> &c, int i) { return c.current(i); },
          "Return the i-th element of the current array.", py::arg("i"))
      .def(
          "__iter__",
          [](EigenCounter<Eigen::VectorXd> &c) {
            return py::make_iterator(c.begin(), c.end());
          },
          py::keep_alive<
              0, 1>() /* Essential: keep object alive while iterator exists */);

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
