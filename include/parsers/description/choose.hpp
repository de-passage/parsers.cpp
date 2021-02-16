#ifndef GUARD_PARSERS_DESCRIPTION_CHOOSE_HPP
#define GUARD_PARSERS_DESCRIPTION_CHOOSE_HPP

#include "./alternative.hpp"
#include "./modifiers.hpp"

#include "../interpreter_traits.hpp"
#include "../interpreters/make_parser.hpp"
#include "../interpreters/object_parser.hpp"

namespace parsers::description {

template <class... Ps>
struct choose
    : modifier<alternative<Ps...>,
               interpreters::make_parser_t<interpreters::object_parser>> {
  using base =
      modifier<alternative<Ps...>,
               interpreters::make_parser_t<interpreters::object_parser>>;

  constexpr choose() noexcept = default;

  template <class... Qs>
  constexpr explicit choose(Qs&&... qs) noexcept
      : base{alternative<Ps...>{std::forward<Qs>(qs)...}} {}

  template <class V,
            std::enable_if_t<
                dpsg::is_template_instance_v<std::decay_t<V>, std::variant>,
                int> = 0>
  [[nodiscard]] constexpr auto operator()(V&& variant) const noexcept {
    return unpack(std::forward<V>(variant),
                  dpsg::feed_t<V, parsers::type_list_t>{},
                  dpsg::feed_t<V, std::index_sequence_for>{});
  }

  template <class It>
  using result_t = std::common_type_t<
      parsers::interpreter_value_type<typename base::interpreter_t, It, Ps>...>;

 private:
  template <std::size_t N, class E, class V>
  static inline constexpr E get(V&& variant) noexcept {
    return static_cast<E>(std::get<N>(std::forward<V>(variant)));
  }

  template <class V, std::size_t... Idx, class... VArgs>
  static inline constexpr std::common_type_t<VArgs...> unpack(
      V&& variant,
      [[maybe_unused]] parsers::type_list_t<VArgs...>,
      [[maybe_unused]] std::index_sequence<Idx...>) {
    using ret_t = std::common_type_t<VArgs...>;
    using func_t = ret_t (*)(V &&);
    constexpr func_t jmp_tbl[sizeof...(VArgs)] = {get<Idx, ret_t, V>...};
    return jmp_tbl[variant.index()](std::forward<V>(variant));
  }
};
template <class... Ps>
choose(Ps&&...) -> choose<detail::remove_cvref_t<Ps>...>;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_CHOOSE_HPP