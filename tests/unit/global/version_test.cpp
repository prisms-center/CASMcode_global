#include "casm/global/version.hh"

#include <regex>

#include "gtest/gtest.h"

using namespace CASM;

namespace test {
std::regex semver_regex() {
  return std::regex(
      R"---((0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?)---");
}
}  // namespace test

TEST(VersionTest, Test1) {
  std::smatch v_match;
  std::regex_match(CASM::version(), v_match, test::semver_regex());

  // Use <major> "." <minor> "." <patch>
  // or <major> "." <minor> "." <patch> "-" <pre-release> using "alpha.1",
  // "beta.1", "beta.2", ...
  EXPECT_EQ(v_match.size(), 6);
  EXPECT_EQ(v_match[1].str(), "2");
  EXPECT_EQ(v_match[2].str(), "2");
  EXPECT_EQ(v_match[3].str(), "0");
  EXPECT_EQ(v_match[4].str(), "");

  std::cout << "CASM::version(): " << CASM::version() << std::endl;
  EXPECT_EQ(CASM::version(), "2.2.0");
  EXPECT_EQ(CASM::version(), casm_global_version());
}
