#include "casm/casm_io/json/jsonParser.hh"

#include <cstdint>
#include <filesystem>
#include <fstream>

#include "casm/misc/CASM_math.hh"

namespace CASM {

/// Functions for converting basic types to/from json

jsonParser &to_json(bool value, jsonParser &json) {
  json.self() = value;
  return json;
}

jsonParser &to_json(int value, jsonParser &json) {
  json.self() = value;
  return json;
}

jsonParser &to_json(unsigned int value, jsonParser &json) {
  json.self() = uint64_t(value);
  return json;
}

jsonParser &to_json(const long int value, jsonParser &json) {
  json.self() = int64_t(value);
  return json;
}

jsonParser &to_json(const unsigned long int value, jsonParser &json) {
  json.self() = uint64_t(value);
  return json;
}

jsonParser &to_json(double value, jsonParser &json) {
  if (value != value) {
    return to_json("nan", json);
  } else if (value == 1.0 / 0.0) {
    return to_json("inf", json);
  } else if (value == -1.0 / 0.0) {
    return to_json("-inf", json);
  } else {
    json.self() = value;
    return json;
  }
}

jsonParser &to_json(const std::string &value, jsonParser &json) {
  json.self() = value;
  return json;
}

jsonParser &to_json(const char *value, jsonParser &json) {
  json.self() = value;
  return json;
}

jsonParser &to_json(const jsonParser &value, jsonParser &json) {
  return json = value;
}

/// Create a jsonParser by reading a file
///
/// This function reads the contents of the file at 'file_path' as if it were
/// JSON. Use 'to_json(file_path.string(), json)' if you only want the path as a
/// string
void to_json(fs::path file_path, jsonParser &json) {
  if (!fs::exists(file_path)) {
    std::stringstream msg;
    msg << "file does not exist: " << file_path;
    throw std::runtime_error(msg.str());
  }
  try {
    json.read(file_path);
  } catch (std::exception &e) {
    std::stringstream msg;
    msg << std::endl;
    msg << "ERROR: Could not parse JSON file: " << file_path << std::endl;
    msg << e.what() << std::endl;
    msg << "Please check your formatting. For "
           "instance, try http://www.jsoneditoronline.org.";
    throw std::runtime_error(msg.str());
  }
}

template <>
bool from_json<bool>(const jsonParser &json) {
  return json.self().get<bool>();
}

template <>
int from_json<int>(const jsonParser &json) {
  return json.self().get<int>();
}

template <>
unsigned int from_json<unsigned int>(const jsonParser &json) {
  return json.self().get<unsigned int>();
}

template <>
long int from_json<long int>(const jsonParser &json) {
  return json.self().get<int64_t>();
}

template <>
unsigned long int from_json<unsigned long int>(const jsonParser &json) {
  return json.self().get<uint64_t>();
}

template <>
double from_json<double>(const jsonParser &json) {
  return json.self().get<double>();
}

template <>
std::string from_json<std::string>(const jsonParser &json) {
  return json.self().get<std::string>();
}

template <>
jsonParser from_json<jsonParser>(const jsonParser &json) {
  return json;
}

void from_json(bool &value, const jsonParser &json) {
  json.self().get_to(value);
}

void from_json(int &value, const jsonParser &json) {
  json.self().get_to(value);
}

void from_json(unsigned int &value, const jsonParser &json) {
  json.self().get_to(value);
}

void from_json(long int &value, const jsonParser &json) {
  json.self().get_to(value);
}

void from_json(unsigned long int &value, const jsonParser &json) {
  json.self().get_to(value);
}

void from_json(double &value, const jsonParser &json) {
  if (json.is_string()) {
    std::string str = json.self().get<std::string>();
    if (str == "nan") {
      value = sqrt(-1.0);
    } else if (str == "inf") {
      value = 1.0 / 0.0;
    } else if (str == "-inf") {
      value = -1.0 / 0.0;
    } else {
      throw std::runtime_error(
          "Expected json real, received string other than 'nan', 'inf', or "
          "'-inf': '" +
          str + "'");
    }
  } else {
    json.self().get_to(value);
  }
}

void from_json(std::string &value, const jsonParser &json) {
  json.self().get_to(value);
}

void from_json(jsonParser &value, const jsonParser &json) { value = json; }

void from_json(fs::path &value, const jsonParser &json) {
  value = json.get<std::string>();
}

// ---- Read/Print JSON  ----------------------------------

void jsonParser::read(std::istream &stream) { stream >> self(); }

void jsonParser::read(const fs::path &file_path) {
  if (!fs::exists(file_path)) {
    std::stringstream msg;
    msg << "file does not exist: " << file_path;
    throw std::runtime_error(msg.str());
  }
  std::ifstream stream(file_path);
  read(stream);
}

std::istream &operator>>(std::istream &stream, jsonParser &json) {
  try {
    json.read(stream);
  } catch (std::exception &e) {
    std::stringstream msg;
    msg << std::endl;
    msg << "ERROR: Could not parse JSON" << std::endl;
    msg << e.what() << std::endl;
    msg << "Please check your formatting. For instance, try "
           "http://www.jsoneditoronline.org.";
    throw std::runtime_error(msg.str());
  }
  return stream;
}

/// Writes json to stream
void jsonParser::print(std::ostream &stream, unsigned int indent,
                       unsigned int prec) const {
  // TODO: formatting prec, single_line_arrays
  stream << self().dump(indent);
};

/// Write json to file
void jsonParser::write(const std::string &file_name, unsigned int indent,
                       unsigned int prec) const {
  std::ofstream file(file_name.c_str());
  print(file, indent, prec);
  file.close();
  return;
}

/// Write json to file
void jsonParser::write(const fs::path &file_path, unsigned int indent,
                       unsigned int prec) const {
  std::ofstream file(file_path);
  print(file, indent, prec);
  file.close();
  return;
}

std::ostream &operator<<(std::ostream &stream, const jsonParser &json) {
  json.print(stream);
  return stream;
}

bool jsonParser::almost_equal(const jsonParser &B, double tol) const {
  if (self().type() != B.self().type()) {
    return false;
  }

  if (is_array()) {
    auto f = [=](const jsonParser &_A, const jsonParser &_B) {
      return _A.almost_equal(_B, tol);
    };
    bool res = (size() == B.size() && std::equal(begin(), end(), B.begin(), f));
    return res;
  } else if (is_obj()) {
    if (size() != B.size()) {
      return false;
    }
    auto A_it = begin();
    auto A_end = end();
    auto B_it = B.begin();
    for (; A_it != A_end; ++A_it, ++B_it) {
      if (A_it.name() != B_it.name() || !A_it->almost_equal(*B_it, tol)) {
        return false;
      }
    }
    return true;
  } else if (is_float()) {
    bool res = CASM::almost_equal(this->get<double>(), B.get<double>(), tol);
    return res;
  } else {
    bool res = (*this == B);
    return res;
  }
}

// ------ Type Checking Methods ------------------------------------

/// Check if null type
bool jsonParser::is_null() const { return self().is_null(); }

/// Check if bool type
bool jsonParser::is_bool() const { return self().is_boolean(); }

/// Check if int type
bool jsonParser::is_int() const { return self().is_number_integer(); }

/// Check if number type (not including int)
bool jsonParser::is_float() const { return self().is_number_float(); }

/// Check if number type (including int)
bool jsonParser::is_number() const { return self().is_number(); }

/// Check if string
bool jsonParser::is_string() const { return self().is_string(); }

/// Check if object type
bool jsonParser::is_obj() const { return self().is_object(); }

/// Check if array type
bool jsonParser::is_array() const { return self().is_array(); }

// ---- Navigate the JSON data: ----------------------------

/// Return a reference to the sub-jsonParser (JSON value) with 'name' if it
/// exists
///   If it does not exist, create it with empty object and return a
///   reference
jsonParser &jsonParser::operator[](const std::string &name) {
  auto it = self().find(name);
  // if 'name' not found, add it as an empty object
  if (it == self().end()) {
    auto result = self().emplace(name, nlohmann::json::object());
    it = result.first;
  }
  return (jsonParser &)*it;
}

/// Return a reference to the sub-jsonParser (JSON value) with 'name' if it
/// exists.
///   Will throw if the 'name' doesn't exist.
const jsonParser &jsonParser::operator[](const std::string &name) const {
  auto it = self().find(name);
  // if 'name' not found, add it and with value 'null'
  if (it == self().end()) {
    throw std::runtime_error("JSON const operator[] access, but " + name +
                             " does not exist");
  }
  return (const jsonParser &)*it;
}

/// Return a reference to the sub-jsonParser (JSON value) with specified
/// relative path
///   Will throw if the 'path' doesn't exist.
///
/// - If 'path' is 'A/B/C', then json[path] is equivalent to json[A][B][C]
/// - If any sub-jsonParser is an array, it will attempt to convert the filename
/// to int
jsonParser &jsonParser::at(const fs::path &path) {
  return const_cast<jsonParser &>(
      static_cast<const jsonParser *>(this)->at(path));
}

/// Return a reference to the sub-jsonParser (JSON value) with specified
/// relative path
///   Will throw if the 'path' doesn't exist.
///
/// - If 'path' is 'A/B/C', then json[path] is equivalent to json[A][B][C]
/// - If any sub-jsonParser is an array, it will attempt to convert the filename
/// to int
const jsonParser &jsonParser::at(const fs::path &path) const {
  if (!path.is_relative()) {
    throw std::invalid_argument(
        "Error in jsonParser::operator[](const fs::path &path): path must be "
        "relative");
  }
  const jsonParser *curr = this;
  for (auto it = path.begin(); it != path.end(); ++it) {
    if (curr->is_array()) {
      int index;
      try {
        index = std::stoi(it->string());
      } catch (std::exception &e) {
        fs::path curr_path;
        for (auto tmp_it = path.begin(); tmp_it != path.end(); ++tmp_it) {
          curr_path /= tmp_it->string();
          if (tmp_it == it) {
            break;
          }
        }

        std::stringstream msg;
        msg << "Error in jsonParser::at: stoi error when attempting to access "
               "array element. "
            << "path: '" << path << "' "
            << "curr_path: '" << curr_path << "'";
        throw std::invalid_argument(msg.str());
      }
      if (curr->size() > index) {
        curr = &((*curr)[index]);
      } else {
        std::stringstream msg;
        msg << "Error in jsonParser::at: attempted to access element outside "
               "of array range. "
            << "path: '" << path << "' "
            << "index: " << index << " "
            << "curr->size(): " << curr->size();
        throw std::invalid_argument(msg.str());
      }
    } else {
      auto res = curr->find(it->string());
      if (res != curr->end()) {
        curr = &((*curr)[it->string()]);
      } else {
        std::stringstream msg;
        msg << "Error in jsonParser::at: key '" << it->string()
            << "' not found at '" << path << "'.";
        throw std::invalid_argument(msg.str());
      }
    }
  }

  return *curr;
}

/// Return a reference to the sub-jsonParser (JSON value) from index 'element'
/// iff jsonParser is a JSON array
jsonParser &jsonParser::operator[](const size_type &element) {
  return (jsonParser &)self()[element];
}

/// Return a const reference to the sub-jsonParser (JSON value) from index
/// 'element' iff jsonParser is a JSON array
const jsonParser &jsonParser::operator[](const size_type &element) const {
  return (const jsonParser &)self()[element];
}

/// Return a reference to the sub-jsonParser (JSON value) from index 'element'
/// iff jsonParser is a JSON array
jsonParser &jsonParser::at(const size_type &element) {
  if (!is_array()) {
    throw std::invalid_argument(
        "Error in jsonParser::at: attempting to access non-array with index");
  }
  if (!(element < size())) {
    throw std::out_of_range("Error in jsonParser::at: out of range");
  }
  return (jsonParser &)self()[element];
}

/// Return a const reference to the sub-jsonParser (JSON value) from index
/// 'element' iff jsonParser is a JSON array
const jsonParser &jsonParser::at(const size_type &element) const {
  if (!is_array()) {
    throw std::invalid_argument(
        "Error in jsonParser::at: attempting to access non-array with index");
  }
  if (!(element < size())) {
    throw std::out_of_range("Error in jsonParser::at: out of range");
  }
  return (const jsonParser &)self()[element];
}

namespace {
/// Return the location at which jsonParser 'A' != 'B' as a fs::path
fs::path find_diff(const jsonParser &A, const jsonParser &B, fs::path diff) {
  auto A_it = A.cbegin();
  auto B_it = B.cbegin();
  while (A_it != A.cend()) {
    if (*A_it != *B_it) {
      if (A.is_obj() && B.is_obj()) {
        return find_diff(*A_it, *B_it, diff / A_it.name());
      } else if (A.is_array() && B.is_array()) {
        std::stringstream ss;
        ss << std::distance(A.cbegin(), A_it);
        return find_diff(*A_it, *B_it, diff / ss.str());
      }
      return diff;
    }
    ++A_it;
    ++B_it;
  }
  return diff;
}

/// Return the location at which jsonParser !A.almost_equal(B, tol) as a
/// fs::path
fs::path find_diff(const jsonParser &A, const jsonParser &B, double tol,
                   fs::path diff) {
  auto A_it = A.cbegin();
  auto B_it = B.cbegin();
  while (A_it != A.cend()) {
    if (!A_it->almost_equal(*B_it, tol)) {
      if (A.is_obj() && B.is_obj()) {
        if (A.size() != B.size()) {
          return diff;
        }
        return find_diff(*A_it, *B_it, tol, diff / A_it.name());
      } else if (A.is_array() && B.is_array()) {
        if (A.size() != B.size()) {
          return diff;
        }
        std::stringstream ss;
        ss << std::distance(A.cbegin(), A_it);
        return find_diff(*A_it, *B_it, tol, diff / ss.str());
      }
      return diff;
    }
    ++A_it;
    ++B_it;
  }
  return diff;
}

}  // namespace

/// Return the location at which jsonParser 'A' != 'B' as a fs::path
fs::path find_diff(const jsonParser &A, const jsonParser &B) {
  return find_diff(A, B, fs::path());
}

/// Return the location at which jsonParser !A.almost_equal(B, tol) as a
/// fs::path
fs::path find_diff(const jsonParser &A, const jsonParser &B, double tol) {
  return find_diff(A, B, tol, fs::path());
}

/// Returns array size if *this is a JSON array, object size if *this is a JSON
/// object, 0 if null, 1 otherwise
jsonParser::size_type jsonParser::size() const { return self().size(); }

/// Returns iterator to beginning of JSON object or JSON array
jsonParser::iterator jsonParser::begin() {
  return iterator(this, self().begin());
}

/// Returns const_iterator to beginning of JSON object or JSON array
jsonParser::const_iterator jsonParser::begin() const { return cbegin(); }

/// Returns const iterator to beginning of const JSON object or JSON array
jsonParser::const_iterator jsonParser::cbegin() const {
  return const_iterator(this, self().begin());
}

/// Returns iterator to end of JSON object or JSON array
jsonParser::iterator jsonParser::end() { return iterator(this, self().end()); }

/// Returns iterator to end of JSON object or JSON array
jsonParser::const_iterator jsonParser::end() const { return cend(); }

/// Returns const_iterator to end of JSON object or JSON array
jsonParser::const_iterator jsonParser::cend() const {
  return const_iterator(this, self().end());
}

/// Return iterator to JSON object value with 'name'
jsonParser::iterator jsonParser::find(const std::string &name) {
  return iterator(this, self().find(name));
}

/// Return const_iterator to JSON object value with 'name'
jsonParser::const_iterator jsonParser::find(const std::string &name) const {
  return const_iterator(this, self().find(name));
}

/// Return iterator to sub-object or element, or 'end' if not found
///
/// - If path.empty(), throw
jsonParser::iterator jsonParser::find_at(const fs::path &path) {
  if (!path.is_relative()) {
    throw std::invalid_argument(
        "Error in jsonParser::operator[](const fs::path &path): path must be "
        "relative");
  }
  if (path.empty()) {
    throw std::invalid_argument(
        "Error in jsonParser::operator[](const fs::path &path): "
        "path must not be empty");
  }
  jsonParser *curr = this;
  jsonParser::iterator res = this->end();
  for (auto it = path.begin(); it != path.end(); ++it) {
    if (curr->is_array()) {
      int index = std::stoi(it->string());
      if (curr->size() > index) {
        res = curr->begin();
        for (int i = 0; i < index; ++i) {
          ++res;
        }
        curr = &(*res);
      } else {
        return this->end();
      }
    } else {
      res = curr->find(it->string());
      if (res != curr->end()) {
        curr = &((*curr)[it->string()]);
      } else {
        return this->end();
      }
    }
  }

  return res;
}

/// Return iterator to sub-object or element, or 'end' if not found
///
/// - If path.empty(), return iterator that dereferences to this, and one
/// increment results in end
jsonParser::const_iterator jsonParser::find_at(const fs::path &path) const {
  return const_cast<jsonParser *>(this)->find_at(path);
}

/// Return true if JSON object contains 'name'
bool jsonParser::contains(const std::string &name) const {
  return find(name) != cend();
}

/// Erase key:value pair from an object
///   Returns the number of elements erased, which will be 0 or 1
jsonParser::size_type jsonParser::erase(const std::string &name) {
  return self().erase(name);
}

// ---- static Methods -------------------------------------

/// Construct a jsonParser from a file containing JSON data
jsonParser jsonParser::parse(const fs::path &path) { return jsonParser(path); }

template <bool IsConst>
jsonParserIterator<IsConst>::jsonParserIterator() {}

template <bool IsConst>
jsonParserIterator<IsConst>::jsonParserIterator(const jsonParserIterator &iter)
    : parser(iter.parser), m_it(iter.m_it) {}

template <bool IsConst>
jsonParserIterator<IsConst> &jsonParserIterator<IsConst>::operator=(
    jsonParserIterator iter) {
  swap(*this, iter);
  return *this;
}

template <bool IsConst>
jsonParserIterator<IsConst>::jsonParserIterator(
    typename jsonParserIterator<IsConst>::pointer j,
    const typename jsonParserIterator<IsConst>::iterator &iter)
    : parser(j), m_it(iter) {}

template <bool IsConst>
typename jsonParserIterator<IsConst>::reference
jsonParserIterator<IsConst>::operator*() const {
  return static_cast<reference>(*m_it);
}

template <bool IsConst>
typename jsonParserIterator<IsConst>::pointer
jsonParserIterator<IsConst>::operator->() const {
  return static_cast<pointer>(&(*m_it));
}

template <bool IsConst>
bool jsonParserIterator<IsConst>::operator==(
    const jsonParserIterator &iter) const {
  return parser == iter.parser && m_it == iter.m_it;
}

template <bool IsConst>
bool jsonParserIterator<IsConst>::operator!=(
    const jsonParserIterator &iter) const {
  return !(*this == iter);
}

template <bool IsConst>
jsonParserIterator<IsConst> &jsonParserIterator<IsConst>::operator++() {
  ++m_it;
  return *this;
}

template <bool IsConst>
jsonParserIterator<IsConst> jsonParserIterator<IsConst>::operator++(int) {
  jsonParserIterator cp(*this);
  ++m_it;
  return cp;
}

template <bool IsConst>
jsonParserIterator<IsConst> &jsonParserIterator<IsConst>::operator--() {
  --m_it;
  return *this;
}

template <bool IsConst>
jsonParserIterator<IsConst> jsonParserIterator<IsConst>::operator--(int) {
  jsonParserIterator<IsConst> cp(*this);
  --m_it;
  return cp;
}

template <bool IsConst>
jsonParserIterator<IsConst>::operator jsonParser::const_iterator() const {
  return jsonParser::const_iterator(parser, m_it);
}

/// When iterating over a JSON object, returns the 'name' of the 'name':value
/// pair the iterator is pointing at
template <bool IsConst>
std::string const &jsonParserIterator<IsConst>::name() const {
  return m_it.key();
}

/// When iterating over a JSON object, returns the 'name' of the 'name':value
/// pair the iterator is pointing at
template <bool IsConst>
std::string const &jsonParserIterator<IsConst>::key() const {
  return m_it.key();
}

template <bool IsConst>
void swap(jsonParserIterator<IsConst> &a, jsonParserIterator<IsConst> &b) {
  using std::swap;

  swap(a.parser, b.parser);
  swap(a.m_it, b.m_it);
}

template class jsonParserIterator<true>;
template class jsonParserIterator<false>;
template void swap<true>(jsonParserIterator<true> &,
                         jsonParserIterator<true> &);
template void swap<false>(jsonParserIterator<false> &,
                          jsonParserIterator<false> &);

}  // namespace CASM
