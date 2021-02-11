#ifndef GUARD_PARSERS_DESCRIPTION_BUILD_HPP
#define GUARD_PARSERS_DESCRIPTION_BUILD_HPP

#include "../interpreters/range_parser.hpp"
#include "./modifiers.hpp"

namespace parsers::description {

template <class D, class T>
struct build
    : modifier<D, interpreters::make_parser_t<interpreters::range_parser>> {
  using base =
      modifier<D, interpreters::make_parser_t<interpreters::range_parser>>;
  template <class U, class E>
  constexpr build(E&& desc, U&& func) noexcept
      : base{std::forward<E>(desc)}, transformer{std::forward<U>(func)} {}

  T transformer;
};
template <class D, class T>
build(D&&, T&&) -> build<detail::remove_cvref_t<D>, detail::remove_cvref_t<T>>;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_BUILD_HPP