#ifndef GUARD_PARSERS_HPP
#define GUARD_PARSERS_HPP

#include "./range.hpp"

#include "./description.hpp"
#include "./dsl.hpp"

#include "./interpreters.hpp"

#include "./additional_dsl.hpp"
#include "./description/dependent_modifiers.hpp"

#include "./interpreter_traits.hpp"
#include "./result_traits.hpp"

#include <iterator>
#include <string_view>
#include <type_traits>

namespace parsers {

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

template <class Descriptor, class... Tr>
[[nodiscard]] constexpr auto match_view(
    Descriptor&& descriptor,
    const std::basic_string<Tr...>& input) noexcept {
  using std::begin;
  const auto eom = end_of_match(std::forward<Descriptor>(descriptor), input);
  const auto b = begin(input);
  return input.substr(0, static_cast<std::size_t>(std::distance(b, eom)));
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

namespace detail {
struct extract_parser_result_t {
  template <class U>
  [[nodiscard]] constexpr typename std::decay_t<U>::second_type operator()(
      U&& pair) const noexcept {
    return std::get<1>(std::forward<U>(pair));
  }
};
constexpr static inline extract_parser_result_t extract_parser_result{};
}  // namespace detail

template <class Description, class T>
constexpr auto parse(Description&& desc, const T& input) noexcept {
  using std::begin, std::end;
  const auto parser =
      parsers::interpreters::make_parser<parsers::interpreters::object_parser>(
          std::forward<Description>(desc));
  return parser(begin(input), end(input)).map(detail::extract_parser_result);
}

}  // namespace parsers

#endif  // GUARD_PARSERS_HPP