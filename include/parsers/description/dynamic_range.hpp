#ifndef GUARD_PARSERS_DESCRIPTION_DYNAMIC_RANGE_HPP
#define GUARD_PARSERS_DESCRIPTION_DYNAMIC_RANGE_HPP

#include "../utility.hpp"
#include "./containers.hpp"

namespace parsers::description {
namespace detail {
template <std::size_t S>
struct static_count {
  constexpr static inline std::size_t count() noexcept { return S; }
};
struct dynamic_count {
  constexpr dynamic_count(std::size_t s) noexcept : _s{s} {}
  [[nodiscard]] constexpr inline std::size_t count() const noexcept {
    return _s;
  }

 private:
  std::size_t _s;
};
}  // namespace detail

template <class N, class P, class C = empty_container<P>>
struct at_least : N, C {
  constexpr at_least() noexcept = default;
  template <class Q,
            std::enable_if_t<!std::is_same_v<std::decay_t<Q>, at_least> &&
                                 std::is_convertible_v<Q, P>,
                             int> = 0>
  constexpr at_least(Q&& q) : N{}, C{std::forward<Q>(q)} {}
  template <
      class Q,
      std::enable_if_t<!std::is_same_v<std::decay_t<Q>, at_least>, int> = 0>
  constexpr at_least(std::size_t n, Q&& q) : C{std::forward<Q>(q)}, N{n} {}

  friend constexpr std::true_type is_dynamic_range_f(const at_least&) noexcept;
};
template <class P, class P1 = detail::remove_cvref_t<P>>
at_least(std::size_t, P&&)
    -> at_least<detail::dynamic_count, P1, container<P1>>;
template <std::size_t S, class P, class C = empty_container<P>>
using at_least_n = at_least<detail::static_count<S>, P, C>;

template <class P, class C = empty_container<P>>
struct many : at_least_n<0, P, C> {
  using base = at_least_n<0, P, C>;
  constexpr many() noexcept = default;
  template <class Q,
            std::enable_if_t<!std::is_same_v<std::decay_t<Q>, many> &&
                                 std::is_constructible_v<base, Q>,
                             int> = 0>
  constexpr explicit many(Q&& q) : base{std::forward<Q>(q)} {}
};
template <class P, class P1 = detail::remove_cvref_t<P>>
many(P&&) -> many<P1, container<P1>>;

template <class P, class C = empty_container<P>>
struct many1 : at_least_n<1, P, C> {
  using base = at_least_n<1, P, C>;
  constexpr many1() noexcept = default;
  template <class Q,
            std::enable_if_t<!std::is_same_v<std::decay_t<Q>, many1> &&
                                 std::is_constructible_v<base, Q>,
                             int> = 0>
  constexpr explicit many1(Q&& q) : base{std::forward<Q>(q)} {}
};
template <class P, class P1 = detail::remove_cvref_t<P>>
many1(P&&) -> many1<P1, container<P1>>;

constexpr std::false_type is_dynamic_range_f(...) noexcept;
template <class T>
using is_dynamic_range = decltype(is_dynamic_range_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_dynamic_range_v = is_dynamic_range<T>::value;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_DYNAMIC_RANGE_HPP
