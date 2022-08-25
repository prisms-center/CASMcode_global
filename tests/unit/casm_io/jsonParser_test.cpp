#include "casm/casm_io/json/jsonParser.hh"

#include <filesystem>

#include "casm/casm_io/container/json_io.hh"
#include "casm/casm_io/json/InputParser_impl.hh"
#include "casm/global/filesystem.hh"
#include "casm/misc/CASM_math.hh"
#include "gtest/gtest.h"
#include "testdir.hh"

using namespace CASM;

std::string json_str =
    R"({
"int" : 34,
"number" : 4.0023,
"string" : "hello",
"bool_true" : true,
"bool_false" : false,
"object" : {
"int" : 34,
"number" : 4.0023,
"string" : "hello",
"bool_true" : true,
"bool_false" : false
},
"uniform_array" : [1, 2, 3, 4],
"mixed_array" : [
"hello",
34,
4.0023,
{"int" : 34, "number" : 4.0023}
]
})";

TEST(jsonParserTest, Basic) {
  jsonParser json = jsonParser::parse(json_str);

  ASSERT_EQ(true, json.is_obj());

  // test jsonParser::get<T>()
  int i;
  from_json(i, json["int"]);
  ASSERT_EQ(i, 34);
  ASSERT_EQ(34, json["int"].get<int>());

  double d;
  from_json(d, json["number"]);
  ASSERT_EQ(d, 4.0023);
  ASSERT_EQ(4.0023, json["number"].get<double>());
}

TEST(jsonParserTest, Basic2) {
  jsonParser json;
  ASSERT_EQ(true, json.is_obj());

  json["something"] = true;
  ASSERT_EQ(true, json.is_obj());
  ASSERT_EQ(true, json["something"].is_bool());

  json = 3;
  ASSERT_EQ(true, json.is_int());

  json = std::string("hello");
  ASSERT_EQ(true, json.is_string());
}

template <typename T>
void test_at(T &json) {
  ASSERT_EQ(json.at("int").template get<int>(), 34);
  ASSERT_THROW(json.at("mistake"), std::invalid_argument);

  ASSERT_EQ(json.at(fs::path("object") / "int").template get<int>(), 34);
  ASSERT_THROW(json.at(fs::path("object") / "mistake"), std::invalid_argument);

  ASSERT_EQ(json.at(fs::path("mixed_array") / "1").template get<int>(), 34);
  ASSERT_THROW(json.at(fs::path("mixed_array") / "10"), std::invalid_argument);

  ASSERT_EQ(json["uniform_array"].at(0).template get<int>(), 1);
  ASSERT_THROW(json["object"].at(0), std::invalid_argument);
  ASSERT_THROW(json["uniform_array"].at(-1), std::out_of_range);
  ASSERT_THROW(json["uniform_array"].at(4), std::out_of_range);
  ASSERT_THROW(json["uniform_array"].at(100), std::out_of_range);
}

TEST(jsonParserTest, At) {
  jsonParser json = jsonParser::parse(json_str);
  test_at(json);
}

TEST(jsonParserTest, ConstAt) {
  const jsonParser json = jsonParser::parse(json_str);
  test_at(json);
}

template <typename T>
void test_find_at(T &json) {
  ASSERT_THROW(json.find_at(fs::path()), std::exception);

  ASSERT_EQ(json.find_at("int")->template get<int>(), 34);
  ASSERT_EQ(json.find_at("mistake") == json.end(), true);

  ASSERT_EQ(json.find_at(fs::path("object") / "int")->template get<int>(), 34);
  ASSERT_EQ(json.find_at(fs::path("object") / "mistake") == json.end(), true);

  ASSERT_EQ(json.find_at(fs::path("mixed_array") / "1")->template get<int>(),
            34);
  ASSERT_EQ(json.find_at(fs::path("mixed_array") / "10") == json.end(), true);
}

TEST(jsonParserTest, FindAt) {
  jsonParser json = jsonParser::parse(json_str);
  test_find_at(json);
}

TEST(jsonParserTest, ConstFindAt) {
  const jsonParser json = jsonParser::parse(json_str);

  test_find_at(json);
}

TEST(jsonParserTest, Get) {
  const jsonParser json = jsonParser::parse(json_str);

  ASSERT_EQ(json["int"].get<int>(), 34);
  ASSERT_THROW(json["int"].get<std::string>(), std::exception);
}

TEST(jsonParserTest, ArrayExtraTrailingComma) {
  std::string json_extra_trailing_comma =
      R"({
"int" : 34,
"number" : 4.0023,
"string" : "hello",
"bool_true" : true,
"bool_false" : false,
"object" : {
"int" : 34,
"number" : 4.0023,
"string" : "hello",
"bool_true" : true,
"bool_false" : false
},
"uniform_array" : [1, 2, 3, 4,],
"mixed_array" : [
"hello",
34,
4.0023,
{"int" : 34, "number" : 4.0023}
]
})";

  ASSERT_THROW(jsonParser::parse(json_extra_trailing_comma), std::exception);
}

TEST(jsonParserTest, ArrayMissingComma) {
  std::string json_missing_comma =
      R"({
"int" : 34,
"number" : 4.0023,
"string" : "hello",
"bool_true" : true,
"bool_false" : false,
"object" : {
"int" : 34,
"number" : 4.0023,
"string" : "hello",
"bool_true" : true,
"bool_false" : false
},
"uniform_array" : [1, 2 3, 4],
"mixed_array" : [
"hello",
34,
4.0023,
{"int" : 34, "number" : 4.0023}
]
})";

  ASSERT_THROW(jsonParser::parse(json_missing_comma), std::exception);
}

TEST(jsonParserTest, FindDiffTest) {
  jsonParser A = jsonParser::parse(json_str);
  jsonParser B{A};

  B["object"]["number"] = B["object"]["number"].get<double>() + 1e-8;

  ASSERT_TRUE(A != B);
  ASSERT_TRUE(A.almost_equal(B, 1e-5));

  fs::path diff_point = find_diff(A, B);
  ASSERT_EQ(diff_point, fs::path("object/number"));
  ASSERT_EQ(A.at(diff_point), 4.0023);

  diff_point = find_diff(A, B, 1e-5);
  ASSERT_TRUE(diff_point.empty());
}

TEST(jsonParserTest, DumpTest) {
  jsonParser json;
  json["a"] = "string";
  json["b"] = "string";
  json["c"] = std::vector<int>({0, 1, 2, 3});
  json["d"] = std::vector<std::vector<int>>({{0, 1, 2}, {0, 1, 2}, {0, 1, 2}});
  json["e"] = std::vector<double>({0.0, 1.0, 2.0, 3.0});
  json["e"][1] = std::vector<double>({0.0, 1.0, 2.0, 3.0});
  json["ee"] = std::vector<double>({0.0, 1.0, 2.0, 3.0});
  json["ee"][0] = std::vector<double>({0.0, 1.0, 2.0, 3.0});
  // json["ee"].set_singleline_array();
  json["ee"][0].set_multiline_array();
  // json["ee"][0].unset_singleline_array();
  json["f"] = std::vector<std::vector<double>>(
      {{0.0, 1.0, 2.0}, {0.0, 1.0, 2.0}, {0.0, 1.0, 2.0}});
  json["g"] = std::vector<bool>({true, false, true, false});
  json["h"] = std::vector<std::vector<bool>>(
      {{true, false, true}, {true, false, true}, {true, false, true}});
  json["i"] = std::vector<std::string>({"aaa", "bbb", "ccc", "ddd"});
  json["i"].set_multiline_array();
  json["j"] = std::vector<std::vector<std::string>>(
      {{"aaa", "bbb", "ccc"}, {"aaa", "bbb", "ccc"}, {"aaa", "bbb", "ccc"}});

  std::cout << json << std::endl;
  EXPECT_EQ(true, true);
}

struct Example {
  Example(double _number, std::string _string)
      : number(_number), string(_string) {}
  double number;
  std::string string;
};

void parse(InputParser<Example> &parser) {
  double number;
  parser.require(number, "number");

  std::string string;
  parser.require(string, "string");

  if (parser.valid()) {
    parser.value = std::make_unique<Example>(number, string);
  }
}

TEST(InputParserTest, SubparseFromFileTest) {
  jsonParser json;
  json["example"] = test::data_file("casm_io", "example_1.json").string();

  ParentInputParser parser(json);
  auto subparser = parser.subparse_from_file<Example>("example");

  EXPECT_EQ(parser.valid(), true);
  EXPECT_EQ(subparser->valid(), true);
  EXPECT_EQ(CASM::almost_equal(subparser->value->number, 12.2), true);
  EXPECT_EQ(subparser->value->string, "abc");

  std::cout << "number: " << subparser->value->number << std::endl;
  std::cout << "string: " << subparser->value->string << std::endl;
}
