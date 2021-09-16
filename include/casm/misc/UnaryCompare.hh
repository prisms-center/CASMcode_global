#ifndef CASM_misc_UnaryCompare
#define CASM_misc_UnaryCompare

#include "casm/misc/type_traits.hh"

namespace CASM {

/// For any binary functor class that accepts two arguments, such as
/// an equals or compare functor, create a unary functor that
/// stores a reference to the LHS value, and a reference to the
/// binary functor, which then only needs to be provided the RHS value.
/// This is useful for cases where one wants to use a binary functor
/// in the context of something like std::find_if
///
/// Example:
/// UnaryCompare_f<BinaryCompare_f>
/// equals_target_element(target_element,args,for,binary,comapre);
/// std::find_if(v.begin(),v.end(),equals_target_element);
template <typename BinaryCompare>
class UnaryCompare_f {
 public:
  using argument_type = notstd::first_argument_type<BinaryCompare>;

  template <typename... CompareArgs>
  UnaryCompare_f(const argument_type &lhs, const CompareArgs &...args)
      : m_lhs(lhs), m_compare_method(args...) {}

  bool operator()(const argument_type &rhs) {
    return m_compare_method(m_lhs, rhs);
  }

 private:
  // TODO: Is having this as a reference too scary? We could just make it a
  // copy;
  const argument_type &m_lhs;
  const BinaryCompare m_compare_method;
};

}  // namespace CASM

#endif
