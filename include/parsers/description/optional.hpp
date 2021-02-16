#ifndef GUARD_PARSERS_DESCRIPTION_OPTIONAL_HPP
#define GUARD_PARSERS_DESCRIPTION_OPTIONAL_HPP

#include "./alternative.hpp"
#include "./guard.hpp"
#include "./modifiers.hpp"

#include "../interpreter_traits.hpp"
#include "../interpreters/make_parser.hpp"
#include "../interpreters/object_parser.hpp"

namespace parsers::description {
template <class T>
struct optional
    : modifier<alternative<T, succeed_t>,
               interpreters::make_parser_t<interpreters::object_parser>> {
  using base =
      modifier<alternative<T, succeed_t>,
               interpreters::make_parser_t<interpreters::object_parser>>;

  constexpr optional() noexcept = default;

  template <
      class U,
      std::enable_if_t<!std::is_same_v<std::decay_t<U>, optional>, int> = 0>
  constexpr explicit optional(U&& u) noexcept
      : base{alternative{std::forward<U>(u), succeed_t{}}} {}

  template <class It>
  using result_t = std::optional<
      parsers::interpreter_value_type<interpreters::object_parser, It, T>>;

  template <
      class V,
      class R = std::optional<std::variant_alternative_t<0, std::decay_t<V>>>>
  constexpr R operator()(V&& variant) const noexcept {
    if (variant.index() == 0) {
      return R{std::get<0>(std::forward<V>(variant))};
    }
    return R{};
  }
};
template <class T>
optional(T&&) -> optional<detail::remove_cvref_t<T>>;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_OPTIONAL_HPP