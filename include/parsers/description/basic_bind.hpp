#ifndef GUARD_PARSERS_DESCRIPTION_BASIC_BIND_HPP
#define GUARD_PARSERS_DESCRIPTION_BASIC_BIND_HPP

#include "./containers.hpp"

namespace parsers::description {
template <class D, class F, class I>
struct basic_bind : container<D> {
  using base = container<D>;
  using interpreter_t = I;

  template <class E, class G>
  constexpr basic_bind(E&& desc, G&& func) noexcept
      : base{std::forward<E>(desc)}, function{std::forward<G>(func)} {}

  F function;
  I interpreter;

  template <class T>
  using result_t = std::invoke_result_t<F, T>;

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