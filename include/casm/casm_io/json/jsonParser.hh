#ifndef CASM_JSONPARSER_HH
#define CASM_JSONPARSER_HH

#include <complex>
#include <exception>

// It is recommended to turn off implicit conversions from JSON
// and leaving them on breaks jsonParser
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "casm/external/nlohmann/json.hpp"
#include "casm/global/filesystem.hh"
#include "casm/misc/CASM_TMP.hh"

namespace CASM {

template <bool IsConst>
class jsonParserIterator;

/**
 * \defgroup jsonParser
 *
 * \brief JSON input/output
 *
 * \ingroup casmIO
 *
 * @{
 */

/// \brief Reading / writing JSON data
///
/// JSON consists of values, which can be of type:
///     object, array, float, int, str, null, bool
///   object: map/dict of <string, JSON value>
///   array: vector/list of JSON values
///
///   value[string] -> value iff object
///   value[int] -> value iff array
///   value.get<type>() -> type must be correct type
///
/// Assumes functions exist for each type T to read/write:
///   jsonParser& to_json( const T &value, jsonParser &json)
///   void from_json( T &value, const jsonParser &json);
///
///   These functions exist for basic types (bool, int, double, std::string),
///   and the containers:
///      Eigen::MatrixXd, CASM::Array, CASM::Matrix, CASM::Vector,
///   So if they also exist for the type being contained,
///   you can read/write the entire object as a JSON array or nested JSON arrays
///
/// ## Simple Usage:
///
/// ### Reading data from JSON file / string / stream:
///
/// \code
/// // read from file
/// jsonParser json;
/// json.read("myfile.json");
/// \endcode
///
/// \code
/// // read from file
/// jsonFile json(fs::path("myfile.json"));
/// \endcode
///
/// \code
/// // parse from JSON formatted string
/// std::string str = "{\"key\":10}";
/// jsonParser json = jsonParser::parse(str);
/// \endcode
///
/// \code
/// // parse from JSON formatted string
/// std::stringstream ss;
/// ss << "{\"key\":10}";
/// jsonParser json = jsonParser::parse(ss);
/// \endcode
///
/// Note:
/// - `jsonParser("myfile.json")` creates a JSON object with string value
///   `"myfile.json"`, it does not read read from a file named `"myfile.json"`.
///
/// ### Ways to get data of type T:
///
/// Note:
/// - In the following, any additional arguments (`...args`) provided are
///   forward to the JSON parsing implementation for the particular type being
///   parsed. Basic types (i.e. bool, int, double, std::string, etc.) do not
///   require any additional arguments.
///
/// \code
/// // assign value
/// T t;
/// from_json(t, json["path"]["to"]["object"]["data"], Args &&...args);
/// \endcode
///
/// \code
/// // construct value
/// T t = json["path"]["to"]["array"][0].get<T>(Args &&...args);
/// \endcode
///
/// \code
/// // assign from value of "key" attribute
/// T t;
/// json["key"].get(t, Args &&...args);
/// \endcode
///
/// \code
/// // only assign if "key" attribute exists
/// T t;
/// json.get_if(t, "key", Args &&...args);
/// \endcode
///
/// \code
/// // construct value from attribute, if "key" attribute exists,
/// // else return 'default_value'
/// T default_value = ...;
/// T t = json.get_if_else(t, "key", default_value, Args &&...args);
/// \endcode
///
/// \code
/// // construct value in std::unique_ptr
/// std::unique_ptr<T> t = json["path"]["to"]["array"][0].make<T>(
///     Args &&...args);
/// \endcode
///
/// \seealso jsonParser::make_if, jsonParser::make_optional,
///     jsonParser::make_if_else, jsonParser::make_else,
///
/// ### Accessing object attributes and array elements:
///
/// - Use `jsonParser::operator[](std::string const&)` for accessing JSON
///   object properties. Non-const version will construct an empty JSON object
///   if the attribute does not already exist; const version will throw.
/// - Use `jsonParser::at(fs::path const&)` for accessing multiple levels of a
///   JSON document
///   - If `fs::path path = "A/B/C"`, then `json[path]` is equivalent to
///     `json[A][B][C]`
///   - If any sub-jsonParser is an array, it will attempt to convert the
///     filename to int
///   - Will throw if does not exist
/// - Use `jsonParser::operator[](size_type const &)` for accessing array
///   elments
/// - Use `jsonParser::find(std::string const&)` to return an iterator.
/// - Use `jsonParser::find_at(fs::path const&)` to access multiple levels of a
///   JSON document.
///
/// ### Check if an attribute with a particular 'name' exists:
///
/// \code
/// if( json.contains("key") ) {
///      json["key"].get(value);
/// }
/// \endcode
///
/// \code
/// auto it = json.find("key");
/// if( it != json.end() ) {
///     value = it->get<T>();
/// }
/// \endcode
///
/// \code
/// auto it = json.find_at("key1/key2/key3");
/// if( it != json.end() ) {
///     value = it->get<T>();
/// }
/// \endcode
///
///
/// ### Format data to JSON:
///
/// \code
/// jsonParser json;
/// T t = ...;
/// to_json(t, json["key"]);
/// \endcode
///
/// \code
/// jsonParser json;
/// T t = ...;
/// json["key"] = t;
/// \endcode
///
/// \code
/// jsonParser json = jsonParser::array();
/// for (int i=0; i<10; ++i) {
///   json.push_back(i);
/// }
/// \endcode
///
/// \code
/// jsonParser json;
/// std::vector<int> v = {0, 1, 2, 3, 4};
/// json.put_array(v.begin(), v.end());
/// \endcode
///
/// \code
/// jsonParser json;
/// int n = 5;
/// std::string value = "value"
/// json.put_array(n, value);
/// \endcode
///
///
/// ### Writing data to a JSON file:
///
/// \code
/// jsonParser json;
/// // ... add data to json ...
/// ofstream file("myfile.json");
/// json.write(file);
/// file.close();
/// \endcode
///
class jsonParser : public nlohmann::json {
 public:
  typedef nlohmann::json::size_type size_type;
  typedef jsonParserIterator<false> iterator;
  typedef jsonParserIterator<true> const_iterator;

  // ---- Constructors  ----------------------------------

  /// Create a new jsonParser, an empty object
  jsonParser() : nlohmann::json(nlohmann::json::object()) {}

  /// Create a new jsonParser, from a nlohmann::json object
  explicit jsonParser(nlohmann::json const &_json) : nlohmann::json(_json) {}

  /// Create a jsonParser from any other object for which 'to_json(t, json)' is
  /// defined
  ///
  /// To parse a std::string as JSON, rather than store it verbatim, use
  /// jsonParser::parse
  template <typename T>
  explicit jsonParser(T &t) {
    to_json(t, *this);
  }

  /// Create a jsonParser from any other object for which 'to_json(t, json)' is
  /// defined
  ///
  /// To parse a std::string as JSON, rather than store it verbatim, use
  /// jsonParser::parse
  template <typename T>
  explicit jsonParser(const T &t) {
    to_json(t, *this);
  }

  /// Formatting directive, when passed as argument to to_json method, will
  /// attempt to format as single, instead of nested array
  struct as_array {};

  /// Formatting directive, when passed as argument to to_json method, will
  /// attempt to format as 'flattest' object (scalar for 1x1x1x..., single array
  /// for nx1x..., etc.
  struct as_flattest {};

  // ---- Read/Print JSON  ----------------------------------

  /// Reads json from the stream
  void read(std::istream &stream);

  /// Reads json from a path
  void read(const fs::path &mypath);

  /// Print json to stream
  void print(std::ostream &stream, unsigned int indent = 2,
             unsigned int prec = 12) const;

  /// Write json to file
  void write(const std::string &file_name, unsigned int indent = 2,
             unsigned int prec = 12) const;

  /// Write json to file
  void write(const fs::path &mypath, unsigned int indent = 2,
             unsigned int prec = 12) const;

  // ---- Value level printing options: ---------------------

  void set_force_column() const {
    // set_force_column is not yet supported with nlohmann::json
  }

  bool operator==(const jsonParser &json) const {
    return (self() == json.self());
  }
  bool operator!=(const jsonParser &json) const {
    return !(self() == json.self());
  }

  bool almost_equal(const jsonParser &B, double tol) const;

  // ------ Type Checking Methods ------------------------------------

  bool is_null() const;
  bool is_bool() const;
  bool is_int() const;
  bool is_float() const;
  bool is_number() const;
  bool is_string() const;
  bool is_obj() const;
  bool is_array() const;

  // ---- Navigate the JSON data: ----------------------------

  // /// Return a reference to the sub-jsonParser (JSON value) with 'name' if it
  // /// exists
  // ///   If it does not exist, create it with an empty JSON object and return
  // a
  // ///   reference to it
  // jsonParser &operator[](const char *name) {
  //   return (*this)[std::string(name)];
  // }
  //
  // /// Return a reference to the sub-jsonParser (JSON value) with 'name' if it
  // /// exists.
  // ///   Will throw if the 'name' doesn't exist.
  // const jsonParser &operator[](const char *name) const {
  //   return (*this)[std::string(name)];
  // }

  /// Return a reference to the sub-jsonParser (JSON value) with 'name' if it
  /// exists
  ///   If it does not exist, create it with an empty JSON object and return a
  ///   reference to it
  jsonParser &operator[](const std::string &name);

  /// Return a reference to the sub-jsonParser (JSON value) with 'name' if it
  /// exists.
  ///   Will throw if the 'name' doesn't exist.
  const jsonParser &operator[](const std::string &name) const;

  /// Return a reference to the sub-jsonParser (JSON value) with specified
  /// relative path
  ///   Will throw if the 'path' doesn't exist.
  ///
  /// - If 'path' is 'A/B/C', then json.at(path) is equivalent to json[A][B][C]
  /// - If any sub-jsonParser is an array, it will attempt to convert the
  /// filename to int
  jsonParser &at(const fs::path &path);

  /// Return a reference to the sub-jsonParser (JSON value) with specified
  /// relative path
  ///   Will throw if the 'path' doesn't exist.
  ///
  /// - If 'path' is 'A/B/C', then json.at(path) is equivalent to json[A][B][C]
  /// - If any sub-jsonParser is an array, it will attempt to convert the
  /// filename to int
  const jsonParser &at(const fs::path &path) const;

  /// Return a reference to the sub-jsonParser (JSON value) from index 'element'
  /// iff jsonParser is a JSON array
  jsonParser &operator[](const size_type &element);

  /// Return a const reference to the sub-jsonParser (JSON value) from index
  /// 'element' iff jsonParser is a JSON array
  const jsonParser &operator[](const size_type &element) const;

  /// Return a reference to the sub-jsonParser (JSON value) from index 'element'
  /// iff jsonParser is a JSON array
  jsonParser &at(const size_type &element);

  /// Return a const reference to the sub-jsonParser (JSON value) from index
  /// 'element' iff jsonParser is a JSON array
  const jsonParser &at(const size_type &element) const;

  /// Returns array size if *this is a JSON array, object size if *this is a
  /// JSON object, 1 otherwise
  size_type size() const;

  /// Returns const_iterator to beginning of JSON object or JSON array
  iterator begin();

  /// Returns iterator to beginning of JSON object or JSON array
  const_iterator begin() const;

  /// Returns iterator to end of JSON object or JSON array
  iterator end();

  /// Returns const_iterator to end of JSON object or JSON array
  const_iterator end() const;

  /// Returns const_iterator to beginning of JSON object or JSON array
  const_iterator cbegin() const;

  /// Returns const_iterator to end of JSON object or JSON array
  const_iterator cend() const;

  /// Return iterator to JSON object value with 'name'
  iterator find(const std::string &name);

  /// Return const_iterator to JSON object value with 'name'
  const_iterator find(const std::string &name) const;

  /// Return iterator to sub-object or element, or 'end' if not found
  jsonParser::iterator find_at(const fs::path &path);

  /// Return iterator to sub-object or element, or 'end' if not found
  jsonParser::const_iterator find_at(const fs::path &path) const;

  /// Return true if JSON object contains 'name'
  bool contains(const std::string &name) const;

  /// Erase key:value pair from an object
  size_type erase(const std::string &name);

  // ---- Data-retrieval Methods -----------------------------------------

  /// Get data from json, using one of several alternatives
  template <typename T, typename... Args>
  T get(Args &&...args) const;

  /// Get data from json, for any type T for which 'void from_json( T &value,
  /// const jsonParser &json, Args... args)' is defined
  ///   Call using: T t; json.get(t);
  template <typename T, typename... Args>
  void get(T &t, Args &&...args) const;

  /// Get data from json, if 'this' contains 'key'
  ///   Returns true if 'key' found, else false
  template <typename T, typename... Args>
  bool get_if(T &t, const std::string &key, Args &&...args) const;

  /// Get data from json, if 'this' contains 'key', else return to
  /// 'default_value'
  template <typename T, typename... Args>
  T get_if_else(const std::string &key, const T &default_value,
                Args &&...args) const;

  /// Get data from json, if 'this' contains 'key', else set to 'default_value'
  ///   Returns true if 'key' found, else false
  template <typename T, typename... Args>
  bool get_else(T &t, const std::string &key, const T &default_value,
                Args &&...args) const;

  /// Get data from json
  template <typename T, typename... Args>
  std::unique_ptr<T> make(Args &&...args) const;

  /// Get data from json
  template <typename T, typename... Args>
  void make(std::unique_ptr<T> &ptr, Args &&...args) const;

  /// Get data from json if key exists
  template <typename T, typename... Args>
  bool make_if(std::unique_ptr<T> &ptr, const std::string &key,
               Args &&...args) const;

  /// Get data from json if key exists, else return empty ptr
  template <typename T, typename... Args>
  std::unique_ptr<T> make_optional(const std::string &key,
                                   Args &&...args) const;

  /// Get data from json if 'this' contains 'key', else return 'default_value'
  template <typename T, typename... Args>
  std::unique_ptr<T> make_if_else(const std::string &key,
                                  std::unique_ptr<T> default_value,
                                  Args &&...args) const;

  /// Get data from json if key exists, else assign default_value
  template <typename T, typename... Args>
  bool make_else(std::unique_ptr<T> &ptr, const std::string &key,
                 std::unique_ptr<T> default_value, Args &&...args) const;

  // ---- Data addition Methods (Overwrites any existing data with same 'name')
  // ---

  /// Puts data of any type T for which 'jsonParser& to_json( const T &value,
  /// jsonParser &json)' is defined
  template <typename T>
  jsonParser &operator=(const T &value);

  /// Puts new valued element at end of array of any type T for which
  /// 'jsonParser& to_json( const T &value, jsonParser &json)' is defined
  template <typename T, typename... Args>
  jsonParser &push_back(const T &value, Args &&...args);

  /// Puts data of any type T for which 'jsonParser& to_json( const T &value,
  /// jsonParser &json)' is defined (same as 'operator=')
  template <typename T>
  jsonParser &put(const T &value);

  /// Puts new empty JSON object
  jsonParser &put_obj() { return *this = object(); }

  /// Puts new JSON object, from iterators over a range of values of type
  /// std::pair<std::string, T>
  template <typename Iterator, typename... Args,
            typename CASM_TMP::enable_if_iterator<Iterator>::type * = nullptr>
  jsonParser &put_obj(Iterator begin, Iterator end, Args &&...args);

  /// Puts new empty JSON array
  jsonParser &put_array() { return *this = array(); }

  /// Puts new size N JSON array of null
  jsonParser &put_array(size_type N) { return *this = array(N); }

  /// Puts new JSON array, using the same value
  template <typename T>
  jsonParser &put_array(size_type N, const T &t);

  /// Puts new JSON array, from iterators
  template <typename Iterator, typename... Args,
            typename CASM_TMP::enable_if_iterator<Iterator>::type * = nullptr>
  jsonParser &put_array(Iterator begin, Iterator end, Args &&...args);

  /// Puts 'null' JSON value
  jsonParser &put_null() { return *this = null(); }

  // ---- static Methods -------------------------------------

  /// Construct a jsonParser from a string containing JSON data
  static jsonParser parse(const std::string &str) {
    std::stringstream ss;
    ss << str;
    return jsonParser(ss);
  }

  /// Construct a jsonParser from a file containing JSON data
  static jsonParser parse(const fs::path &path);

  /// Construct a jsonParser from a stream containing JSON data
  static jsonParser parse(std::istream &stream) { return jsonParser(stream); }

  /// Returns an empty json object
  static jsonParser object() { return jsonParser(); }

  /// Puts new JSON object, from iterators over a range of values of type
  /// std::pair<std::string, T>
  template <typename Iterator>
  static jsonParser object(Iterator begin, Iterator end) {
    jsonParser json;
    return json.put_obj(begin, end);
  }

  /// Returns an empty json array
  static jsonParser array() { return jsonParser(nlohmann::json::array()); }

  /// Returns a size N json array null
  static jsonParser array(size_type N) {
    jsonParser json;
    return json.put_array(N, null());
  }

  /// Puts new JSON array, using the same value
  template <typename T>
  static jsonParser array(size_type N, const T &t) {
    jsonParser json;
    return json.put_array(N, t);
  }

  /// Puts new JSON array, from iterators
  template <typename Iterator>
  static jsonParser array(
      Iterator begin, Iterator end,
      typename CASM_TMP::enable_if_iterator<Iterator>::type * = nullptr) {
    jsonParser json;
    return json.put_array(begin, end);
  }

  /// Returns a null JSON value
  static jsonParser null() { return jsonParser(nlohmann::json()); }

  nlohmann::json &self() { return (nlohmann::json &)*this; }
  nlohmann::json const &self() const { return (nlohmann::json &)*this; }

 private:
  // nlohmann::json m_self;
};

std::ostream &operator<<(std::ostream &stream, const jsonParser &json);
std::istream &operator>>(std::istream &stream, jsonParser &json);

/// To JSON for basic types
jsonParser &to_json(bool value, jsonParser &json);
jsonParser &to_json(int value, jsonParser &json);
jsonParser &to_json(unsigned int value, jsonParser &json);
jsonParser &to_json(long int value, jsonParser &json);
jsonParser &to_json(unsigned long int value, jsonParser &json);
jsonParser &to_json(double value, jsonParser &json);
jsonParser &to_json(const std::string &value, jsonParser &json);
jsonParser &to_json(const char *value, jsonParser &json);
jsonParser &to_json(const jsonParser &value, jsonParser &json);

/// From JSON for basic types
template <typename T>
T from_json(const jsonParser &json);

/// Make from JSON for basic types
template <typename T>
std::unique_ptr<T> make_from_json(const jsonParser &json);

template <>
bool from_json<bool>(const jsonParser &json);
template <>
int from_json<int>(const jsonParser &json);
template <>
unsigned int from_json<unsigned int>(const jsonParser &json);
template <>
long int from_json<long int>(const jsonParser &json);
template <>
unsigned long int from_json<unsigned long int>(const jsonParser &json);
template <>
double from_json<double>(const jsonParser &json);
template <>
std::string from_json<std::string>(const jsonParser &json);
template <>
jsonParser from_json<jsonParser>(const jsonParser &json);

/// From JSON for basic types
void from_json(bool &value, const jsonParser &json);
void from_json(int &value, const jsonParser &json);
void from_json(unsigned int &value, const jsonParser &json);
void from_json(long int &value, const jsonParser &json);
void from_json(unsigned long int &value, const jsonParser &json);
void from_json(double &value, const jsonParser &json);
void from_json(std::string &value, const jsonParser &json);
void from_json(jsonParser &value, const jsonParser &json);
void from_json(std::istream &stream, const jsonParser &json);
void from_json(fs::path &value, const jsonParser &json);

/// Create a jsonParser from a stream
inline void to_json(std::istream &stream, jsonParser &json) {
  try {
    json.read(stream);
  } catch (std::exception &e) {
    std::stringstream msg;
    msg << e.what() << std::endl;
    msg << "ERROR: Could not read JSON. Please check your formatting. For "
           "instance, try http://www.jsoneditoronline.org.";
    throw std::runtime_error(msg.str());
  }
}

/// Create a jsonParser by reading a file
void to_json(fs::path file_path, jsonParser &json);

/// To JSON for complex
template <typename T>
jsonParser &to_json(const std::complex<T> &value, jsonParser &json) {
  json = jsonParser::object();
  json["real"] = value.real();
  json["imag"] = value.imag();
  return json;
}

/// From JSON for complex
template <typename T>
void from_json(std::complex<T> &value, const jsonParser &json) {
  value = std::complex<T>(json["real"].get<T>(), json["imag"].get<T>());
}

/// To JSON for std::pair<std::string, T>
template <typename Key, typename T>
jsonParser &to_json(const std::pair<Key, T> &value, jsonParser &json);

/// From JSON for std::pair<std::string, T>
template <typename Key, typename T>
void from_json(std::pair<Key, T> &value, const jsonParser &json);

/// \brief Helper struct for constructing objects that need additional data
///     beyond what is in the JSON data
///
/// \code jsonParser::get<T>(Args&&...args) \endcode is equivalent to:
/// - \code jsonConstructor<T>::from_json(*this, args...) \endcode
///
/// This struct can be specialized to create new jsonConstructor<T>::from_json
/// as needed.
template <typename ReturnType>
struct jsonConstructor {
  /// \brief Default from_json is equivalent to \code
  /// CASM::from_json<ReturnType>(json) \endcode
  static ReturnType from_json(const jsonParser &json) {
    return CASM::from_json<ReturnType>(json);
  }
};

/// \brief Helper struct for constructing objects (in std::unique_ptr) that need
///     additional data beyond what is in the JSON data
///
/// \code jsonParser::make<T>(Args&&...args) \endcode is equivalent to:
/// - \code jsonMake<T>::make_from_json(*this, args...) \endcode
///
/// This struct can be specialized to create new jsonMake<T>::make_from_json
/// as needed.
template <typename ValueType>
struct jsonMake {
  /// \brief Default make_from_json is equivalent to \code
  /// CASM::make_from_json<ValueType>(json) \endcode
  static std::unique_ptr<ValueType> make_from_json(const jsonParser &json) {
    return CASM::make_from_json<ValueType>(json);
  }
};

/// Default works if T::T() and 'void from_json(T&, const jsonParser&)' exist
template <typename T>
T from_json(const jsonParser &json) {
  T value;
  from_json(value, json);
  return value;
}

/// Default uses 'from_json<T>(const jsonParser&)' with copy constructor
template <typename T>
std::unique_ptr<T> make_from_json(const jsonParser &json) {
  return std::unique_ptr<T>(new T(from_json<T>(json)));
}

// /// Make from JSON for basic types
// template<typename T>
// void make_from_json(std::unique_ptr<T>& ptr, const jsonParser &json) {
//   ptr = jsonMake<T>::make_from_json(json);
// }

/// Make from JSON for basic types
template <typename T, typename... Args>
void make_from_json(std::unique_ptr<T> &ptr, const jsonParser &json,
                    Args &&...args) {
  ptr = jsonMake<T>::make_from_json(json, std::forward<Args>(args)...);
}

/// Return the location at which jsonParser 'A' != 'B' as a
/// std::filesystem::path
fs::path find_diff(const jsonParser &A, const jsonParser &B);

/// Return the location at which jsonParser !A.almost_equal(B, tol) as a
/// std::filesystem::path
fs::path find_diff(const jsonParser &A, const jsonParser &B, double tol);

/// jsonParser bidirectional Iterator class
///   Can iterate over a JSON object or JSON array or JSON value (though this is
///   only one value) When iterating over a JSON object, can return the 'name'
///   of the current 'name':value pair being pointed at
template <bool IsConst>
class jsonParserIterator {
 public:
  typedef std::forward_iterator_tag iterator_category;
  typedef typename std::conditional<IsConst, nlohmann::json::const_iterator,
                                    nlohmann::json::iterator>::type iterator;
  typedef int difference_type;
  typedef typename std::conditional<IsConst, const jsonParser, jsonParser>::type
      value_type;
  typedef value_type &reference;
  typedef value_type *pointer;

  jsonParserIterator();

  jsonParserIterator(const jsonParserIterator &iter);

  jsonParserIterator &operator=(jsonParserIterator iter);

  jsonParserIterator(pointer j, const iterator &iter);

  reference operator*() const;

  pointer operator->() const;

  bool operator==(const jsonParserIterator &iter) const;

  bool operator!=(const jsonParserIterator &iter) const;

  jsonParserIterator &operator++();

  jsonParserIterator operator++(int);

  jsonParserIterator &operator--();

  jsonParserIterator operator--(int);

  operator jsonParser::const_iterator() const;

  /// When iterating over a JSON object, returns the 'name' of the 'name':value
  /// pair the iterator is pointing at
  std::string const &name() const;

  /// When iterating over a JSON object, returns the 'name' of the 'name':value
  /// pair the iterator is pointing at
  std::string const &key() const;

  template <bool _IsConst>
  friend void swap(jsonParserIterator<_IsConst> &a,
                   jsonParserIterator<_IsConst> &b);

 private:
  pointer parser;

  nlohmann::json::value_t type;

  iterator m_it;

  int val_iter;
};

/// Puts new valued element at end of array of any type T for which 'jsonParser&
/// to_json( const T &value, jsonParser &json)' is defined
template <typename T, typename... Args>
jsonParser &jsonParser::push_back(const T &value, Args &&...args) {
  jsonParser json;
  self().push_back(to_json(value, json, std::forward<Args>(args)...));
  return *this;
}

/// Get data from json, using one of several alternatives
///
/// Use for any type T for which the either of the following is specialized
/// (they are called in the following order):
/// - \code
///   template<typename T>
///   template<typename...Args>
///   T jsonConstructor<T>::from_json(const jsonParser& json, Args&&...args);
///   \endcode
/// - \code
///   template<typename T>
///   T from_json(const jsonParser &json);
///   \endcode
/// If neither is specialized, then this is equivalent to:
/// - \code
///   T value;
///   from_json(value, *this);
///   return value;
///   \endcode
///
template <typename T, typename... Args>
T jsonParser::get(Args &&...args) const {
  return jsonConstructor<T>::from_json(*this, std::forward<Args>(args)...);
}

/// Get data from json
///
/// This is equivalent to:
/// \code
/// from_json(t, *this, std::forward<Args>(args)...);
/// \endcode
///
template <typename T, typename... Args>
void jsonParser::get(T &t, Args &&...args) const {
  from_json(t, *this, std::forward<Args>(args)...);
}

/// Get data from json if key exists
///
/// If 'key' exists, this is equivalent to:
/// \code
/// find(key)->get(t, std::forward<Args>(args)...);
/// \endcode
///
template <typename T, typename... Args>
bool jsonParser::get_if(T &t, const std::string &key, Args &&...args) const {
  auto it = find(key);
  if (it != cend()) {
    it->get(t, std::forward<Args>(args)...);
    return true;
  }
  return false;
}

/// Get data from json if key exists, else return default_value
///
/// If 'key' exists, this is equivalent to:
/// \code
/// find(key)->get<T>(std::forward<Args>(args)...);
/// \endcode
///
template <typename T, typename... Args>
T jsonParser::get_if_else(const std::string &key, const T &default_value,
                          Args &&...args) const {
  auto it = find(key);
  if (it != end()) {
    return it->get<T>(std::forward<Args>(args)...);
  } else {
    return default_value;
  }
}

/// Get data from json if key exists, else assign default_value
///
/// If 'key' exists, this is equivalent to:
/// \code
/// find(key)->get(t, std::forward<Args>(args)...);
/// \endcode
///
template <typename T, typename... Args>
bool jsonParser::get_else(T &t, const std::string &key, const T &default_value,
                          Args &&...args) const {
  auto it = find(key);
  if (it != cend()) {
    it->get(t, std::forward<Args>(args)...);
    return true;
  }

  t = default_value;
  return false;
}

/// Get data from json, using one of several alternatives
///
/// Use for any type T for which the either of the following is specialized
/// (they are called in the following order):
/// - \code
///   template<typename T>
///   template<typename...Args>
///   std::unique_ptr<T> jsonMake<T>::make_from_json(const jsonParser& json,
///   Args&&...args); \endcode
/// - \code
///   template<typename T>
///   std::unique_ptr<T> make_from_json(const jsonParser &json);
///   \endcode
/// If neither is specialized, then this is equivalent to:
/// - \code
///   return std::unique_ptr<T>(new T(from_json<T>(json)));
///   \endcode
///
template <typename T, typename... Args>
std::unique_ptr<T> jsonParser::make(Args &&...args) const {
  return jsonMake<T>::make_from_json(*this, std::forward<Args>(args)...);
}

/// Get data from json
///
/// This is equivalent to:
/// \code
/// make_from_json(ptr, *this, std::forward<Args>(args)...);
/// \endcode
///
template <typename T, typename... Args>
void jsonParser::make(std::unique_ptr<T> &ptr, Args &&...args) const {
  make_from_json(ptr, *this, std::forward<Args>(args)...);
}

/// Get data from json if key exists
///
/// If 'key' exists, this is equivalent to:
/// \code
/// find(key)->make(ptr, std::forward<Args>(args)...);
/// \endcode
///
template <typename T, typename... Args>
bool jsonParser::make_if(std::unique_ptr<T> &ptr, const std::string &key,
                         Args &&...args) const {
  auto it = find(key);
  if (it != cend()) {
    it->make(ptr, std::forward<Args>(args)...);
    return true;
  }
  return false;
}

/// Get data from json if key exists, else return empty ptr
///
/// If 'key' exists, this is equivalent to:
/// \code
/// find(key)->make(std::forward<Args>(args)...);
/// \endcode
///
template <typename T, typename... Args>
std::unique_ptr<T> jsonParser::make_optional(const std::string &key,
                                             Args &&...args) const {
  auto it = find(key);
  if (it != cend()) {
    return it->make(std::forward<Args>(args)...);
  }
  return std::unique_ptr<T>();
}

/// Get data from json if 'this' contains 'key', else return 'default_value'
///
/// If 'key' exists, this is equivalent to:
/// \code
/// find(key)->make(std::forward<Args>(args)...);
/// \endcode
///
template <typename T, typename... Args>
std::unique_ptr<T> jsonParser::make_if_else(const std::string &key,
                                            std::unique_ptr<T> default_value,
                                            Args &&...args) const {
  auto it = find(key);
  if (it != cend()) {
    return it->make(std::forward<Args>(args)...);
  }
  return std::move(default_value);
}

/// Get data from json if key exists, else assign default_value
///
/// If 'key' exists, this is equivalent to:
/// \code
/// find(key)->make(ptr, std::forward<Args>(args)...);
/// \endcode
///
template <typename T, typename... Args>
bool jsonParser::make_else(std::unique_ptr<T> &ptr, const std::string &key,
                           std::unique_ptr<T> default_value,
                           Args &&...args) const {
  auto it = find(key);
  if (it != cend()) {
    it->make(ptr, std::forward<Args>(args)...);
    return true;
  }

  ptr = std::move(default_value);
  return false;
}

/// Puts data of any type T for which 'jsonParser& to_json( const T &value,
/// jsonParser &json)' is defined (same as operator=)
template <typename T>
jsonParser &jsonParser::put(const T &value) {
  return to_json(value, *this);
}

/// Puts new JSON object, from iterators over a range of values of type
/// std::pair<std::string, T>
template <typename Iterator, typename... Args,
          typename CASM_TMP::enable_if_iterator<Iterator>::type *>
jsonParser &jsonParser::put_obj(Iterator begin, Iterator end, Args &&...args) {
  *this = object();
  for (auto it = begin; it != end; ++it) {
    to_json(it->second, (*this)[it->first], std::forward<Args>(args)...);
  }
  return *this;
}

/// Puts new JSON array, using the same value
template <typename T>
jsonParser &jsonParser::put_array(size_type N, const T &t) {
  *this = array();
  for (auto i = 0; i < N; ++i) {
    push_back(t);
  }
  return *this;
}

/// Puts new JSON array, from iterators
template <typename Iterator, typename... Args,
          typename CASM_TMP::enable_if_iterator<Iterator>::type *>
jsonParser &jsonParser::put_array(Iterator begin, Iterator end,
                                  Args &&...args) {
  *this = array();
  for (auto it = begin; it != end; ++it) {
    push_back(*it, std::forward<Args>(args)...);
  }
  return *this;
}

/// Puts data of any type T for which 'jsonParser& to_json( const T &value,
/// jsonParser &json)' is defined
template <typename T>
jsonParser &jsonParser::operator=(const T &value) {
  return to_json(value, *this);
}

/// To JSON for std::pair<std::string, T> and other convertible types
template <typename Key, typename T>
jsonParser &to_json(const std::pair<Key, T> &value, jsonParser &json) {
  json = jsonParser::object();
  return json[value.first] = value.second;
}

/// From JSON for std::pair<std::string, T>
template <typename Key, typename T>
void from_json(std::pair<Key, T> &value, const jsonParser &json) {
  auto it = json.begin();
  value = std::make_pair<Key, T>(it.name(), *it);
}

/// Create pair/value json object without intermediate temporary
template <typename T>
jsonParser json_pair(const std::string &key, const T &value) {
  jsonParser tjson;
  tjson[key] = value;
  return tjson;
}

/** @} */
}  // namespace CASM

#endif
