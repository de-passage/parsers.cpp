#ifndef GUARD_PARSERS_HPP
#define GUARD_PARSERS_HPP

#include "./dsl.hpp"

#include "./customization_points.hpp"

#include "./description.hpp"
#include "./matcher.hpp"
#include "./object_parser.hpp"
#include "./range_parser.hpp"

#include <iterator>
#include <type_traits>

namespace parsers {

namespace interpreters {
namespace detail {
using ::parsers::customization_points::parsers_interpreters_make_parser;

template <class Traits>
struct make_parser_t : Traits {
  template <class T>
  [[nodiscard]] constexpr auto operator()(T&& descriptor) const noexcept {
    return parsers_interpreters_make_parser(std::forward<T>(descriptor), *this);
  }
};
}  // namespace detail

template <class T>
using make_parser_t = detail::make_parser_t<T>;

template <class T>
constexpr static inline make_parser_t<T> make_parser{};
}  // namespace interpreters

namespace detail {
template <class Descriptor, class T>
[[nodiscard]] constexpr auto match(Descriptor&& descriptor,
                                   const T& input) noexcept {
  using std::begin;
  using std::end;
  const auto matcher = interpreters::make_parser<interpreters::matcher>(
      std::forward<Descriptor>(descriptor));
  auto b = begin(input);
  auto e = end(input);
  return matcher(b, e);
}
}  // namespace detail

template <class Descriptor, class T>
[[nodiscard]] constexpr bool match(Descriptor&& descriptor,
                                   const T& input) noexcept {
  return detail::match(std::forward<Descriptor>(descriptor), input).has_value();
}

template <class Descriptor, class T>
[[nodiscard]] constexpr auto end_of_match(Descriptor&& descriptor,
                                          const T& input) noexcept {
  using std::begin;
  return detail::match(std::forward<Descriptor>(descriptor), input)
      .value_or(begin(input));
}

template <class Descriptor, class T>
[[nodiscard]] constexpr auto match_span(Descriptor&& descriptor,
                                        const T& input) noexcept {
  using std::begin;
  return std::pair{begin(input),
                   end_of_match(std::forward<Descriptor>(descriptor), input)};
}

}  // namespace parsers

#endif  // GUARD_PARSERS_HPP