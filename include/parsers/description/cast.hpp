#ifndef GUARD_PARSERS_DESCRIPTION_CAST_HPP
#define GUARD_PARSERS_DESCRIPTION_CAST_HPP

#include "../interpreters/object_parser.hpp"
#include "./modifiers.hpp"

namespace parsers::description {

template <class T, class P>
struct cast
    : modifier<P, interpreters::make_parser_t<interpreters::object_parser>> {
  using base =
      modifier<P, interpreters::make_parser_t<interpreters::object_parser>>;

  constexpr cast() noexcept = default;

  template <class U, class Q>
  constexpr cast([[maybe_unused]] type_t<U> tag, Q&& parser)
      : base{std::forward<Q>(parser)} {}

  template <class I>
  using result_t = T;

  template <class U, std::enable_if_t<!std::is_lvalue_reference_v<U>, int> = 0>
  constexpr T&& operator()(U&& input) const noexcept {
    return static_cast<T&&>(std::move(input));
  }

  template <class U>
  constexpr T& operator()(U& input) const noexcept {
    return static_cast<T&>(input);
  }
};
template <class T, class P>
cast(type_t<T>, P&&) -> cast<T, detail::remove_cvref_t<P>>;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_CAST_HPP