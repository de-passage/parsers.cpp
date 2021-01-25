#ifndef GUARD_PARSERS_HPP
#define GUARD_PARSERS_HPP

#include <iterator>
#include <type_traits>

#include "./description.hpp"
#include "./matcher.hpp"

namespace parsers {
namespace detail {
template <class Descriptor, class T>
[[nodiscard]] constexpr auto match(Descriptor descriptor,
                                   const T& input) noexcept {
  using std::begin;
  using std::end;
  const auto matcher = interpreters::make_matcher(descriptor);
  auto b = begin(input);
  auto e = end(input);
  return matcher(b, e);
}
}  // namespace detail

template <class Descriptor, class T>
[[nodiscard]] constexpr bool match(Descriptor descriptor,
                                   const T& input) noexcept {
  return detail::match(descriptor, input).has_value();
}

template <class Descriptor, class T>
[[nodiscard]] constexpr auto end_of_match(Descriptor descriptor,
                                          const T& input) noexcept {
  using std::begin;
  return detail::match(descriptor, input).value_or(begin(input));
}

template <class Descriptor, class T>
[[nodiscard]] constexpr auto match_span(Descriptor descriptor,
                                        const T& input) noexcept {
  using std::begin;
  return std::pair{begin(input), end_of_match(descriptor, input)};
}

}  // namespace parsers

#endif  // GUARD_PARSERS_HPP