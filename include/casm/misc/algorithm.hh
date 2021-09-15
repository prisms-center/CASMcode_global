#ifndef CASM_algorithm
#define CASM_algorithm

#include <algorithm>
#include <vector>

#include "casm/global/definitions.hh"

namespace CASM {

using std::find;

/// \brief Equivalent to std::find(begin, end, value), but with custom
/// comparison
template <typename Iterator, typename T, typename BinaryCompare>
Iterator find(Iterator begin, Iterator end, const T &value, BinaryCompare q) {
  return std::find_if(begin, end, [&value, q](const T &other) -> bool {
    return q(value, other);
  });
}

/// \brief Equivalent to std::distance(begin, std::find(begin, end, value))
template <typename Iterator, typename T>
Index find_index(Iterator begin, Iterator end, const T &value) {
  return std::distance(begin, std::find(begin, end, value));
}

/// \brief Equivalent to std::distance(begin, find(begin, end, value, q))
template <typename Iterator, typename T, typename BinaryCompare>
Index find_index(Iterator begin, Iterator end, const T &value,
                 BinaryCompare q) {
  return std::distance(begin, find(begin, end, value, q));
}

/// \brief Equivalent to std::distance(container.begin(),
/// find(container.begin(), container.end(), value,q))
template <typename Container, typename T, typename BinaryCompare>
Index find_index(const Container &container, const T &value, BinaryCompare q) {
  return std::distance(container.begin(),
                       find(container.begin(), container.end(), value, q));
}

/// \brief Equivalent to std::distance(container.begin(),
/// std::find(container.begin(), container.end(), value))
template <typename Container, typename T>
Index find_index(const Container &container, const T &value) {
  return std::distance(container.begin(),
                       std::find(container.begin(), container.end(), value));
}

/// \brief Equivalent to std::distance(begin, std::find_if(begin, end, p))
template <typename Iterator, typename UnaryPredicate>
Index find_index_if(Iterator begin, Iterator end, UnaryPredicate p) {
  return std::distance(begin, std::find_if(begin, end, p));
}

/// \brief Equivalent to std::distance(container.begin(),
/// std::find_if(container.begin(), container.end(), p))
template <typename Container, typename UnaryPredicate>
Index find_index_if(const Container &container, UnaryPredicate p) {
  return std::distance(container.begin(),
                       std::find_if(container.begin(), container.end(), p));
}

/// \brief Equivalent to std::distance(begin, std::find_if_not(begin, end, q))
template <typename Iterator, typename UnaryPredicate>
Index find_index_if_not(Iterator begin, Iterator end, UnaryPredicate q) {
  // Please use -Werror=return-type in your compiler flags
  return std::distance(begin, std::find_if_not(begin, end, q));
}

/// \brief Equivalent to std::distance(container.begin(),
/// std::find_if_not(container.begin(), container.end(), p))
template <typename Container, typename UnaryPredicate>
Index find_index_if_not(const Container &container, UnaryPredicate q) {
  return std::distance(container.begin(),
                       std::find_if_not(container.begin(), container.end(), q));
}

/// \brief Equivalent to container.end() != std::find(container.begin(),
/// container.end(), value)
template <typename Container, typename T>
bool contains(const Container &container, const T &value) {
  return container.end() !=
         std::find(container.begin(), container.end(), value);
}

/// \brief Equivalent to container.end() != find(container.begin(),
/// container.end(), value, q)
template <typename Container, typename T, typename BinaryCompare>
bool contains(const Container &container, const T &value, BinaryCompare q) {
  return container.end() != find(container.begin(), container.end(), value, q);
}

/// \brief Equivalent to container.end() != std::find_if(container.begin(),
/// container.end(), p)
template <typename Container, typename UnaryPredicate>
bool contains_if(const Container &container, UnaryPredicate p) {
  return container.end() != std::find_if(container.begin(), container.end(), p);
}

/// \brief Equivalent to container.end() != std::find_if_not(container.begin(),
/// container.end(), q)
template <typename Container, typename UnaryPredicate>
bool contains_if_not(const Container &container, UnaryPredicate q) {
  return container.end() !=
         std::find_if_not(container.begin(), container.end(), q);
}

/// \brief Returns true if each elements of 'values' is contained in 'container'
template <typename Container1, typename Container2>
bool contains_all(const Container1 &container, const Container2 &values) {
  for (auto const &v : values)
    if (!contains(container, v)) return false;

  return true;
}

/// \brief Returns true if each elements of 'values' is contained in
/// 'container', using comparison functor 'q'
template <typename Container1, typename Container2, typename BinaryCompare>
bool contains_all(const Container1 &container, const Container2 &values,
                  BinaryCompare q) {
  for (auto const &v : values)
    if (!contains(container, v, q)) return false;

  return true;
}

template <typename Container>
typename Container::value_type sum(
    const Container &container, typename Container::value_type init_val = 0) {
  for (const auto &val : container) {
    init_val += val;
  }
  return init_val;
}

///\brief Return pointer one past end of vector. Equivalent to
/// convainer.data()+container.size()
template <typename T>
T *end_ptr(std::vector<T> &container) {
  return container.data() + container.size();
}

///\brief Return const pointer one past end of vector. Equivalent to
/// convainer.data()+container.size()
template <typename T>
T const *end_ptr(std::vector<T> const &container) {
  return container.data() + container.size();
}

struct is_any_of {
  is_any_of(std::string _str) : str(_str) {}
  bool operator()(char ch) const {
    return std::find(str.begin(), str.end(), ch) != str.end();
  }
  std::string str;
};

template <typename SequenceT, typename UnaryPredicate>
SequenceT trim_copy_if(SequenceT const &seq, UnaryPredicate is_space) {
  auto begin = std::begin(seq);
  auto end = std::end(seq);
  auto tbegin = begin;
  for (; tbegin != end; ++tbegin) {
    if (!is_space(*tbegin)) {
      break;
    }
  }
  auto tend = end;
  for (; tend != begin;) {
    --tend;
    if (!is_space(*tend)) {
      break;
    }
  }
  return SequenceT(tbegin, tend);
}

enum class empty_token_policy {drop, keep};

const empty_token_policy drop_empty_tokens = empty_token_policy::drop;
const empty_token_policy keep_empty_tokens = empty_token_policy::drop;

struct char_separator {

  char_separator(char const* _dropped_delims,
                 char const* _kept_delims = "",
                 empty_token_policy _empty_tokens = drop_empty_tokens):
      dropped_delims(_dropped_delims),
      kept_delims(_kept_delims),
      empty_tokens(_empty_tokens) {}

 private:
  is_any_of dropped_delims;
  is_any_of kept_delims;
  empty_token_policy empty_tokens;
};

struct tokenizer {

  typedef std::vector<std::string>::const_iterator iterator;

  tokenizer(std::string _expression,
            char_separator _sep) {}

  iterator begin() const {
    return m_tokens.begin();
  }

  iterator cbegin() const {
    return m_tokens.begin();
  }

  iterator end() const {
    return m_tokens.end();
  }

  iterator cend() const {
    return m_tokens.end();
  }

 private:
  std::vector<std::string> m_tokens;
};

}  // namespace CASM

#endif
