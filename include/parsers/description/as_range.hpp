#ifndef GUARD_PARSERS_DESCRIPTION_AS_STRING_VIEW_HPP
#define GUARD_PARSERS_DESCRIPTION_AS_STRING_VIEW_HPP

#include "../range.hpp"
#include "./modifiers.hpp"

#include "../interpreters/make_parser.hpp"
#include "../interpreters/range_parser.hpp"

namespace parsers::description {
template <class T>
struct as_range
    : modifier<T, interpreters::make_parser_t<interpreters::range_parser>> {
  using base =
      modifier<T, interpreters::make_parser_t<interpreters::range_parser>>;

  constexpr as_range() noexcept = default;

  template <class U,
            std::enable_if_t<!std::is_same_v<std::decay_t<U>, T>, int> = 0>
  constexpr explicit as_range(U&& u) noexcept : base{std::forward<U>(u)} {}

  template <class It>
  using result_t =
      decltype(parsers::range{std::declval<It>(), std::declval<It>()});

  template <class P>
  constexpr auto operator()(P&& pair) const noexcept {
    return _build(std::get<0>(std::forward<P>(pair)),
                  std::get<1>(std::forward<P>(pair)));
  }

 private:
  template <class ItB, class ItE>
  constexpr result_t<std::decay_t<ItB>> _build(ItB&& beg,
                                               ItE&& end) const noexcept {
    return result_t<std::decay_t<ItB>>{std::forward<ItB>(beg),
                                       std::forward<ItE>(end)};
  }
};
template <class T>
as_range(T&&) -> as_range<detail::remove_cvref_t<T>>;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_AS_STRING_VIEW_HPP