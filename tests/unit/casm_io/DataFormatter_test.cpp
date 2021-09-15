#include "autotools.hh"
#include "casm/casm_io/dataformatter/DataFormatterTools_impl.hh"
#include "casm/casm_io/dataformatter/DataFormatter_impl.hh"
#include "gtest/gtest.h"

using namespace CASM;

namespace DataFormatterTest_impl {

struct Object {
  Object(int _a, double _b) : a(_a), b(_b) {}

  int a;
  double b;
};
}  // namespace DataFormatterTest_impl

TEST(DataFormatterTest, Test1) {
  using namespace DataFormatterTest_impl;
  DataFormatter<Object> formatter;

  GenericDatumFormatter<int, Object> get_a(
      "get_a", "Returns Object::a", [](Object const &obj) { return obj.a; });

  GenericDatumFormatter<double, Object> get_b(
      "get_b", "Returns Object::b", [](Object const &obj) { return obj.b; });

  formatter.push_back(get_a, get_b);
}
