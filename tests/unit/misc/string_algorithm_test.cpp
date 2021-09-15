#include "casm/misc/string_algorithm.hh"

#include "gtest/gtest.h"

using namespace CASM;

TEST(TrimCopyIfTest, Test1) {
  std::string expr("  something, something, something  ");
  std::string result = trim_copy_if(expr, is_any_of(" "));
  std::string expected("something, something, something");
  EXPECT_EQ(result, expected);
}

TEST(TrimCopyIfTest, Test2) {
  std::string expr(" ( something, something, something ) ");
  std::string result = trim_copy_if(expr, is_any_of(" ()"));
  std::string expected("something, something, something");
  EXPECT_EQ(result, expected);
}

TEST(TrimCopyIfTest, Test3) {
  std::string expr(" ( something, (something), something ) ");
  std::string result = trim_copy_if(expr, is_any_of(" ()"));
  std::string expected("something, (something), something");
  EXPECT_EQ(result, expected);
}

TEST(TokenizerTest, Test1) {
  std::string expr("  something, something, something  ");
  char_separator sep(" ,");
  tokenizer tok(expr, sep);
  std::vector<std::string> result(tok.begin(), tok.end());
  std::vector<std::string> expected({"something", "something", "something"});
  EXPECT_EQ(result, expected);
}

TEST(TokenizerTest, Test2) {
  std::string expr("  something, something, something  ");
  char_separator sep(" ", ",");
  tokenizer tok(expr, sep);
  std::vector<std::string> result(tok.begin(), tok.end());
  std::vector<std::string> expected(
      {"something", ",", "something", ",", "something"});
  EXPECT_EQ(result, expected);
}

TEST(TokenizerTest, Test3) {
  std::string expr("  something, something, something  ");
  char_separator sep(" ", ",", keep_empty_tokens);
  tokenizer tok(expr, sep);
  std::vector<std::string> result(tok.begin(), tok.end());
  std::vector<std::string> expected({"", "", "something", ",", "", "something",
                                     ",", "", "something", "", ""});
  EXPECT_EQ(result, expected);
}

TEST(TokenizerTest, Test4) {
  std::string expr(";;Hello|world||-foo--bar;yow;baz|");
  char_separator sep("-;", "|", keep_empty_tokens);
  tokenizer tok(expr, sep);
  std::vector<std::string> result(tok.begin(), tok.end());
  std::vector<std::string> expected({"", "", "Hello", "|", "world", "|", "",
                                     "|", "", "foo", "", "bar", "yow", "baz",
                                     "|", ""});
  EXPECT_EQ(result, expected);
}

TEST(TokenizerTest, Test5) {
  std::string expr("");
  char_separator sep(" ");
  tokenizer tok(expr, sep);
  std::vector<std::string> result(tok.begin(), tok.end());
  std::vector<std::string> expected({});
  EXPECT_EQ(result, expected);
}

TEST(TokenizerTest, Test6) {
  std::string expr(" ");
  char_separator sep(" ");
  tokenizer tok(expr, sep);
  std::vector<std::string> result(tok.begin(), tok.end());
  std::vector<std::string> expected({});
  EXPECT_EQ(result, expected);
}

TEST(TokenizerTest, Test7) {
  std::string expr(" ");
  char_separator sep(" ", "", keep_empty_tokens);
  tokenizer tok(expr, sep);
  std::vector<std::string> result(tok.begin(), tok.end());
  std::vector<std::string> expected({"", ""});
  EXPECT_EQ(result, expected);
}

TEST(TokenizerTest, Test8) {
  std::string expr(",");
  char_separator sep(" ", ",", keep_empty_tokens);
  tokenizer tok(expr, sep);
  std::vector<std::string> result(tok.begin(), tok.end());
  std::vector<std::string> expected({"", ",", ""});
  EXPECT_EQ(result, expected);
}
