#ifndef GUARD_PARSERS_DESCRIPTION_BUILD_HPP
#define GUARD_PARSERS_DESCRIPTION_BUILD_HPP

#include <type_traits>
#include "../interpreters/range_parser.hpp"
#include "./modifiers.hpp"

namespace parsers::description {

template <class D, class T, class R = void>
struct build
    : modifier<D, interpreters::make_parser_t<interpreters::range_parser>> {
  using base =
      modifier<D, interpreters::make_parser_t<interpreters::range_parser>>;

  constexpr build() noexcept = default;

  template <class U, class E>
  constexpr build(E&& desc, U&& func) noexcept
      : base{std::forward<E>(desc)}, transformer{std::forward<U>(func)} {}

  T transformer;

  template <class I>
  using result_t = typename std::conditional_t<std::is_same_v<R, void>,
                                               std::invoke_result<T, I, I>,
                                               type_t<R>>::type;

  template <class It>
  constexpr auto operator()(It beg, It end) const noexcept {
    return transformer(beg, end);
  }

  template <class P>
  constexpr auto operator()(P&& pair) const noexcept {
    return transformer(std::get<0>(std::forward<P>(pair)),
                       std::get<1>(std::forward<P>(pair)));
  }
};
template <class D, class T>
build(D&&, T&&) -> build<detail::remove_cvref_t<D>, detail::remove_cvref_t<T>>;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_BUILD_HPP