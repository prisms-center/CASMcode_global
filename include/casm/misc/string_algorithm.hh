#ifndef CASM_misc_string_algorithm
#define CASM_misc_string_algorithm

#include <algorithm>
#include <string>
#include <vector>

#include "casm/global/definitions.hh"

namespace CASM {

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
      ++tend;
      break;
    }
  }
  return SequenceT(tbegin, tend);
}

enum class empty_token_policy { drop, keep };

const empty_token_policy drop_empty_tokens = empty_token_policy::drop;
const empty_token_policy keep_empty_tokens = empty_token_policy::keep;

struct char_separator {
  char_separator(char const *_dropped_delims, char const *_kept_delims = "",
                 empty_token_policy _empty_tokens = drop_empty_tokens)
      : dropped_delims(_dropped_delims),
        kept_delims(_kept_delims),
        empty_tokens(_empty_tokens) {}

  is_any_of dropped_delims;
  is_any_of kept_delims;
  empty_token_policy empty_tokens;
};

struct tokenizer {
  typedef std::vector<std::string>::const_iterator iterator;

  tokenizer(std::string expression, char_separator sep) {
    auto begin = expression.begin();
    auto end = expression.end();
    if (begin == end) {
      return;
    }

    auto it = begin;
    while (it != end) {
      if (sep.dropped_delims(*it) || sep.kept_delims(*it)) {
        if (begin != it || sep.empty_tokens == keep_empty_tokens) {
          m_tokens.emplace_back(begin, it);
        }
        begin = it;
        ++begin;
      }
      if (sep.kept_delims(*it)) {
        m_tokens.emplace_back(1, *it);
      }
      ++it;
    }
    if (begin != end || sep.empty_tokens == keep_empty_tokens) {
      m_tokens.emplace_back(begin, end);
    }
  }

  iterator begin() const { return m_tokens.begin(); }

  iterator cbegin() const { return m_tokens.begin(); }

  iterator end() const { return m_tokens.end(); }

  iterator cend() const { return m_tokens.end(); }

 private:
  std::vector<std::string> m_tokens;
};

}  // namespace CASM

#endif
