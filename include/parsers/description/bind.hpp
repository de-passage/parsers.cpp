#ifndef GUARD_PARSERS_DESCRIPTION_OBJECT_BIND_HPP
#define GUARD_PARSERS_DESCRIPTION_OBJECT_BIND_HPP

#include "./basic_bind.hpp"

#include "../interpreters/make_parser.hpp"
#include "../interpreters/object_parser.hpp"

namespace parsers::description {
template <class D, class F>
struct bind
    : basic_bind<D,
                 F,
                 interpreters::make_parser_t<interpreters::object_parser>> {
  using base =
      basic_bind<D,
                 F,
                 interpreters::make_parser_t<interpreters::object_parser>>;

  template <class E, class G>
  constexpr bind(E&& desc, G&& func) noexcept
      : base{std::forward<E>(desc), std::forward<G>(func)} {}
};
template <class D, class F>
bind(D&&, F&&) -> bind<detail::remove_cvref_t<D>, detail::remove_cvref_t<F>>;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_OBJECT_BIND_HPP