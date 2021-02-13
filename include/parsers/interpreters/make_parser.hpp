#ifndef GUARD_PARSER_INTERPRETERS_MAKE_PARSER_HPP
#define GUARD_PARSER_INTERPRETERS_MAKE_PARSER_HPP

#include "./customization_points.hpp"

namespace parsers::interpreters {

namespace detail {
using ::parsers::customization_points::parsers_interpreters_make_parser;

template <class Traits>
struct make_parser_t : Traits {
  constexpr make_parser_t() noexcept = default;
  template <
      class T,
      std::enable_if_t<std::is_convertible_v<T, Traits> &&
                           !std::is_same_v<std::decay_t<T>, make_parser_t>,
                       int> = 0>
  constexpr make_parser_t(T&& t) noexcept : Traits{std::forward<T>(t)} {}

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
}  // namespace parsers::interpreters
#endif  // GUARD_PARSER_INTERPRETERS_MAKE_PARSER_HPP