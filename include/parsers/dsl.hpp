#ifndef GUARD_PARSERS_DSL_HPP
#define GUARD_PARSERS_DSL_HPP

#include <type_traits>
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

namespace detail {
struct is_zero : description::satisfy<is_zero> {
  template <class C>
  [[nodiscard]] constexpr bool operator()(C c) const noexcept {
    return c == static_cast<C>(0);
  }
};
}  // namespace detail

using eos_t = description::either<description::end_t, detail::is_zero>;
constexpr static inline eos_t eos{};

template <class T>
struct fail_t : description::fail_t<fail_t<T>> {
  template <class U, std::enable_if_t<!std::is_same_v<std::decay_t<U>, fail_t>>>
  constexpr inline fail_t(U&& msg) : _message{std::forward<U>(msg)} {}
  [[nodiscard]] constexpr inline const T& message() const noexcept {
    return _message;
  }

 private:
  T _message;
};

template <>
struct fail_t<void> : description::fail_t<fail_t<void>> {
  template <class M>
  constexpr inline fail_t<std::remove_reference_t<std::remove_cv_t<M>>>
  operator()(M&& message) const noexcept {
    return fail_t<std::remove_reference_t<std::remove_cv_t<M>>>{
        std::forward<M>(message)};
  }

  [[nodiscard]] constexpr static inline const void* message() noexcept {
    return nullptr;
  }
};
constexpr static inline fail_t<void> fail;

constexpr static inline parsers::description::self_t self;

namespace detail {}  // namespace detail

struct fix_t {
  template <class D>
  constexpr auto operator()(D&& descriptor) const noexcept {
    return description::fixed<std::decay_t<D>>(std::forward<D>(descriptor));
  }
} constexpr static inline fix;

}  // namespace parsers::dsl

namespace parsers::customization_points {
template <class C, class T, std::enable_if_t<std::is_integral_v<C>, int> = 0>
constexpr auto parsers_interpreters_make_parser(C c, T&& interpreter) noexcept {
  return std::forward<T>(interpreter)(description::dynamic_character<C>{c});
}
template <class C,
          std::size_t S,
          class T,
          std::enable_if_t<std::is_integral_v<C>, int> = 0>
constexpr auto parsers_interpreters_make_parser(C (&c)[S],
                                                T&& interpreter) noexcept {
  return std::forward<T>(interpreter)(
      description::static_string<C>{c, static_cast<C*>(c) + S - 1});
}
}  // namespace parsers::customization_points

#endif  // GUARD_PARSERS_DSL_HPP