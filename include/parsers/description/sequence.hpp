#ifndef GUARD_PARSERS_DESCRIPTION_SEQUENCE_HPP
#define GUARD_PARSERS_DESCRIPTION_SEQUENCE_HPP

#include "../utility.hpp"
#include "./containers.hpp"

namespace parsers::description {

template <class A, class B>
struct both : detail::make_indexed_sequence<A, B> {
  using base = detail::make_indexed_sequence<A, B>;
  constexpr both() = default;
  template <class T, class U>
  constexpr both(T&& t, U&& u) noexcept
      : base{std::forward<T>(t), std::forward<U>(u)} {}

  friend constexpr std::true_type is_sequence_f(const both&) noexcept;
};

template <class A,
          class B,
          class A1 = detail::remove_cvref_t<A>,
          class B1 = detail::remove_cvref_t<B>>
both(A&&, B&&) -> both<A1, B1>;

template <class... Ss>
struct sequence
    : detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...> {
  using base = detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...>;

  static_assert(base::sequence_length > 0, "Empty sequence not allowed");
  constexpr sequence() noexcept = default;
  template <class... Ts,
            std::enable_if_t<
                std::conjunction_v<std::is_constructible<container<Ss>, Ts>...>,
                int> = 0>
  constexpr explicit sequence(Ts&&... ts) noexcept
      : base{std::forward<Ts>(ts)...} {}

  friend constexpr std::true_type is_sequence_f(const sequence&) noexcept;
};
template <class A, class... Args>
sequence(A&&, Args&&...)
    -> sequence<detail::remove_cvref_t<A>, detail::remove_cvref_t<Args>...>;

constexpr std::false_type is_sequence_f(...) noexcept;
template <class T>
using is_sequence = decltype(is_sequence_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_sequence_v = is_sequence<T>::value;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_SEQUENCE_HPP