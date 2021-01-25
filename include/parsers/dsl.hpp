#ifndef GUARD_PARSERS_DSL_HPP
#define GUARD_PARSERS_DSL_HPP

#include "./description.hpp"

namespace parsers::dsl {

template <class A, class B>
[[nodiscard]] constexpr auto operator|([[maybe_unused]] A&& left,
                                       [[maybe_unused]] B&& right) noexcept {
  return description::either{std::forward<A>(left), std::forward<B>(right)};
}

template <class A, class B>
[[nodiscard]] constexpr auto operator+([[maybe_unused]] A&& left,
                                       [[maybe_unused]] B&& right) noexcept {
  return description::both{std::forward<A>(left), std::forward<B>(right)};
}

constexpr description::static_string<char> operator""_s(
    const char* str,
    unsigned long long size) noexcept {
  return description::static_string<char>{str, str + size};
}

constexpr static inline description::end_t end{};
constexpr static inline description::any_t any{};
constexpr static inline description::succeed_t succeed{};
}  // namespace parsers::dsl

#endif  // GUARD_PARSERS_DSL_HPP