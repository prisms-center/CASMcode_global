#include "casm/misc/CASM_math.hh"

#include "casm/casm_io/container/stream_io.hh"
#include "gtest/gtest.h"

using namespace CASM;

TEST(IndexToKComb, Test1) {
  std::vector<Index> kcomb = index_to_kcombination(Index(0), Index(4));
  std::vector<Index> expected = {3, 2, 1, 0};
  EXPECT_EQ(kcomb, expected);
}