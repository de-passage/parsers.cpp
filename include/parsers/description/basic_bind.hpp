#ifndef GUARD_PARSERS_DESCRIPTION_BASIC_BIND_HPP
#define GUARD_PARSERS_DESCRIPTION_BASIC_BIND_HPP

#include "./containers.hpp"

#include "../result_traits.hpp"

namespace parsers::description {
template <class D, class F, class I>
struct basic_bind : container<D> {
  using base = container<D>;
  using interpreter_t = I;

  constexpr basic_bind() noexcept = default;

  template <class E, class G>
  constexpr basic_bind(E&& desc, G&& func) noexcept
      : base{std::forward<E>(desc)}, function{std::forward<G>(func)} {}

  F function;
  I interpreter;

  using initial_interpreted_type =
      std::invoke_result_t<interpreter_t, typename base::parser_t>;
  template <class ItB, class ItE>
  using initial_result_type =
      std::invoke_result_t<initial_interpreted_type, ItB, ItE>;

  template <class ItB, class ItE>
  using initial_value_type = typename parsers::result_traits<
      initial_result_type<ItB, ItE>>::value_type;

  template <class ItB, class ItE>
  using final_parser_type =
      std::invoke_result_t<F, initial_value_type<ItB, ItE>>;

  template <class J, class ItB, class ItE>
  using final_interpreted_type =
      std::invoke_result_t<J, final_parser_type<ItB, ItE>>;

  template <class J, class ItB, class ItE>
  using final_result_type =
      std::invoke_result_t<final_interpreted_type<J, ItB, ItE>, ItB, ItE>;

  template <class T>
  constexpr auto operator()(T&& value) const noexcept {
    return function(std::forward<T>(value));
  }

  friend constexpr std::true_type is_bind_f(const basic_bind&) noexcept;

  constexpr inline auto interpret() const {
    return interpreter(base::parser());
  }
};

constexpr std::false_type is_bind_f(...) noexcept;
template <class T>
using is_bind = decltype(is_bind_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_bind_v = is_bind<T>::value;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_BIND_HPP