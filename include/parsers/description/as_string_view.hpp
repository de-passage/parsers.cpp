#ifndef GUARD_PARSERS_DESCRIPTION_AS_STRING_VIEW_HPP
#define GUARD_PARSERS_DESCRIPTION_AS_STRING_VIEW_HPP

#include "./modifiers.hpp"

#include "../interpreters/make_parser.hpp"
#include "../interpreters/range_parser.hpp"

namespace parsers::description {
template <class T>
struct as_string_view
    : modifier<T, interpreters::make_parser_t<interpreters::range_parser>> {
  using base =
      modifier<T, interpreters::make_parser_t<interpreters::range_parser>>;

  constexpr as_string_view() noexcept = default;

  template <class U,
            std::enable_if_t<!std::is_same_v<std::decay_t<U>, T>, int> = 0>
  constexpr explicit as_string_view(U&& u) noexcept
      : base{std::forward<U>(u)} {}

  template <class It>
  using result_t =
      std::basic_string_view<typename std::iterator_traits<It>::value_type>;

  template <class ItB, class ItE>
  constexpr result_t<std::decay_t<ItB>> operator()(ItB&& beg,
                                                   ItE&& end) const noexcept {
    return result_t<std::decay_t<ItB>>{
        &*std::forward<ItB>(beg),
        static_cast<std::size_t>(std::distance(beg, end))};
  }
};
template <class T>
as_string_view(T&&) -> as_string_view<detail::remove_cvref_t<T>>;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_AS_STRING_VIEW_HPP