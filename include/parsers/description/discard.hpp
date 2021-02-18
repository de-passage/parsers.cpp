#ifndef GUARD_PARSERS_DESCRIPTION_DISCARD_HPP
#define GUARD_PARSERS_DESCRIPTION_DISCARD_HPP

#include "./modifiers.hpp"

#include "../interpreters/make_parser.hpp"
#include "../interpreters/matcher.hpp"

namespace parsers::description {

template <class T>
struct discard
    : modifier<T, interpreters::make_parser_t<interpreters::matcher>> {
  using base = modifier<T, interpreters::make_parser_t<interpreters::matcher>>;

  template <class... Us,
            std::enable_if_t<std::is_constructible_v<base, Us...>, int> = 0>
  constexpr explicit discard(Us&&... us) : base{std::forward<Us>(us)...} {}

  template <class... As>
  constexpr empty operator()([[maybe_unused]] As&&...) const noexcept {
    return empty{};
  }

  template <class I>
  using result_t = empty;
};
template <class T>
discard(T&&) -> discard<detail::remove_cvref_t<T>>;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_DISCARD_HPP