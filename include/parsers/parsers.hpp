#ifndef GUARD_PARSERS_HPP
#define GUARD_PARSERS_HPP

#include "./dsl.hpp"

#include "./customization_points.hpp"

#include "./description.hpp"
#include "./description/ascii.hpp"

#include "./matcher.hpp"
#include "./object_parser.hpp"
#include "./range_parser.hpp"

#include <iterator>
#include <string_view>
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
[[nodiscard]] constexpr auto match_view(Descriptor&& descriptor,
                                        const T& input) noexcept {
  using std::begin;
  using iterator = decltype(begin(input));
  using value_type = typename std::iterator_traits<iterator>::value_type;
  const auto eom = end_of_match(std::forward<Descriptor>(descriptor), input);
  const auto b = begin(input);
  return std::basic_string_view<value_type>{
      b, static_cast<std::size_t>(std::distance(b, eom))};
}

template <class Descriptor, class T>
[[nodiscard]] constexpr std::ptrdiff_t match_length(Descriptor&& descriptor,
                                                    const T& input) noexcept {
  using std::begin;
  return end_of_match(std::forward<Descriptor>(descriptor), input) -
         begin(input);
}

template <class Descriptor, class T>
[[nodiscard]] constexpr bool match_full(Descriptor&& descriptor,
                                        const T& input) noexcept {
  using std::end;
  return end_of_match(std::forward<Descriptor>(descriptor), input) ==
         end(input);
}

template <class Descriptor, class T>
constexpr auto parse_range(Descriptor&& desc, const T& input) noexcept {
  using std::begin;
  using std::end;
  const auto range_parser =
      parsers::interpreters::make_parser<parsers::interpreters::range_parser>(
          std::forward<Descriptor>(desc));
  return range_parser(begin(input), end(input));
}

template <class Descriptor, class T>
constexpr auto parse(Descriptor&& desc, const T& input) noexcept {
  using std::begin, std::end;
  const auto parser =
      parsers::interpreters::make_parser<parsers::interpreters::object_parser>(
          std::forward<Descriptor>(desc));
  return parser(begin(input), end(input)).map([](auto&& pair) {
    return std::get<1>(std::forward<decltype(pair)>(pair));
  });
}

}  // namespace parsers

#endif  // GUARD_PARSERS_HPP