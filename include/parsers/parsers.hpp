#ifndef GUARD_PARSERS_HPP
#define GUARD_PARSERS_HPP

#include <iterator>
#include <type_traits>
#include "./description.hpp"
#include "./matcher.hpp"

namespace parsers {
namespace detail {
template <class T, class... Args>
using one_of = std::disjunction<std::is_same<T, Args>...>;

template <class T>
struct remove_pointer_const {
  using type = T;
};
template <class T>
struct remove_pointer_const<const T*> {
  using type = T*;
};
template <class T>
using remove_pointer_const_t = typename remove_pointer_const<T>::type;
}  // namespace detail

template <class T>
void show_me();

template <class Descriptor, class T>
[[nodiscard]] constexpr bool match(Descriptor descriptor,
                                   const T& input) noexcept {
  using std::begin;
  using std::end;
  const auto matcher = interpreters::make_matcher(descriptor);
  auto b = begin(input);
  auto e = end(input);
  using end_t = detail::remove_pointer_const_t<decltype(e)>;
  if constexpr (detail::one_of<end_t, char*, wchar_t*>::value) {
    --e;
  }
  return matcher(b, e).has_value();
}
}  // namespace parsers

#endif  // GUARD_PARSERS_HPP