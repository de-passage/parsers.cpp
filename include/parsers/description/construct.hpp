#ifndef GUARD_PARSERS_DESCRIPTION_CONSTRUCT_HPP
#define GUARD_PARSERS_DESCRIPTION_CONSTRUCT_HPP

#include "./modifiers.hpp"
#include "./sequence.hpp"

#include "../interpreters/make_parser.hpp"
#include "../interpreters/object_parser.hpp"

namespace parsers::description {
template <class T, class... Ps>
struct construct
    : modifier<sequence<Ps...>,
               interpreters::make_parser_t<interpreters::object_parser>> {
  using base =
      modifier<sequence<Ps...>,
               interpreters::make_parser_t<interpreters::object_parser>>;

  constexpr construct() noexcept = default;

  template <class... Us>
  constexpr explicit construct([[maybe_unused]] type_t<T>, Us&&... u) noexcept
      : base{sequence{std::forward<Us>(u)}...} {}

  template <class It>
  using result_t = T;

  template <class U,
            std::enable_if_t<
                dpsg::is_template_instance_v<std::decay_t<U>, std::tuple>,
                int> = 0>
  constexpr T operator()(U&& tpl) const noexcept {
    return instanciate(
        std::forward<U>(tpl),
        dpsg::feed_t<std::decay_t<U>, std::index_sequence_for>{});
  }

  template <class U,
            std::enable_if_t<
                !dpsg::is_template_instance_v<std::decay_t<U>, std::tuple> &&
                    std::is_convertible_v<U, T>,
                int> = 0>
  constexpr T operator()(U&& tpl) const noexcept {
    return T{std::forward<U>(tpl)};
  }

 private:
  template <class U, std::size_t... S>
  constexpr T instanciate(U&& tpl, [[maybe_unused]] std::index_sequence<S...>)
      const noexcept {
    return T{std::get<S>(std::forward<U>(tpl))...};
  }
};
template <class D, class T>
construct(type_t<D>, T&&) -> construct<D, detail::remove_cvref_t<T>>;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_AS_STRING_VIEW_HPP