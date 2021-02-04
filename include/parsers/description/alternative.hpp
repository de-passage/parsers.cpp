#ifndef GUARD_PARSERS_DESCRIPTION_ALTERNATIVE_HPP
#define GUARD_PARSERS_DESCRIPTION_ALTERNATIVE_HPP

#include "../utility.hpp"
#include "./containers.hpp"

namespace parsers::description {

template <class A, class B>
struct either : detail::make_indexed_sequence<A, B> {
  using base = detail::make_indexed_sequence<A, B>;
  constexpr either() = default;
  template <class T, class U>
  constexpr either(T&& t, U&& u) noexcept
      : base{std::forward<T>(t), std::forward<U>(u)} {}

  friend constexpr std::true_type is_alternative_f(const either&) noexcept;
};

template <class A,
          class B,
          class A1 = detail::remove_cvref_t<A>,
          class B1 = detail::remove_cvref_t<B>>
either(A&&, B&&) -> either<A1, B1>;

template <class... Ss>
struct alternative
    : detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...> {
  using base = detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...>;

  static_assert(base::sequence_length > 0, "Empty alternative not allowed");
  constexpr alternative() noexcept = default;
  template <class... Ts,
            std::enable_if_t<
                std::conjunction_v<std::is_constructible<container<Ss>, Ts>...>,
                int> = 0>
  constexpr explicit alternative(Ts&&... ts) noexcept
      : base{std::forward<Ts>(ts)...} {}

  friend constexpr std::true_type is_alternative_f(const alternative&) noexcept;
};
template <class A, class... Args>
alternative(A&&, Args&&...)
    -> alternative<detail::remove_cvref_t<A>, detail::remove_cvref_t<Args>...>;

constexpr std::false_type is_alternative_f(...) noexcept;

template <class T>
using is_alternative = decltype(is_alternative_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_alternative_v = is_alternative<T>::value;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_ALTERNATIVE_HPP