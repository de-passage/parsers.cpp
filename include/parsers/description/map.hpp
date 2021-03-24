#ifndef GUARD_PARSERS_DESCRIPTION_MAP_HPP
#define GUARD_PARSERS_DESCRIPTION_MAP_HPP

#include "../interpreters/make_parser.hpp"
#include "../interpreters/object_parser.hpp"
#include "./modifiers.hpp"

#include <type_traits>

namespace parsers::description {

template <class D, class T, class R = void>
struct map
    : modifier<D, interpreters::make_parser_t<interpreters::object_parser>> {
  using base =
      modifier<D, interpreters::make_parser_t<interpreters::object_parser>>;

  constexpr map() noexcept = default;

  template <class E, class U>
  constexpr map(E&& desc, U&& func) noexcept
      : base{std::forward<E>(desc)}, modifier{std::forward<U>(func)} {}

  T modifier;

  template <class I>
  using result_t = typename std::conditional_t<
      std::is_same_v<R, void>,
      std::invoke_result<T, interpreters::object_parser::object_t<I, D>>,
      type_t<R>>::type;

  template <class U>
  constexpr auto operator()(U&& obj) const noexcept {
    return modifier(std::forward<U>(obj));
  }
};
template <class D, class T>
map(D&&, T&&) -> map<detail::remove_cvref_t<D>, detail::remove_cvref_t<T>>;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_MAP_HPP