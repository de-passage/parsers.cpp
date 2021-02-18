#ifndef GUARD_PARSERS_DSL_HPP
#define GUARD_PARSERS_DSL_HPP

#include <type_traits>
#include "./description.hpp"

namespace parsers::dsl {

namespace detail {
template <class T>
using with_sequence =
    std::enable_if_t<parsers::description::is_sequence_v<T>, int>;
template <class T>
using with_alternative =
    std::enable_if_t<parsers::description::is_alternative_v<T>, int>;
template <class T>
using without_sequence =
    std::enable_if_t<!parsers::description::is_sequence_v<T>, int>;
template <class T>
using without_alternative =
    std::enable_if_t<!parsers::description::is_alternative_v<T>, int>;

template <template <class...> class R,
          std::size_t... Is,
          std::size_t... Js,
          class T,
          class U>
constexpr auto combine(T&& t,
                       U&& u,
                       std::index_sequence<Is...>,
                       std::index_sequence<Js...>) noexcept {
  return R{std::forward<T>(t).template parser<Is>()...,
           std::forward<U>(u).template parser<Js>()...};
}

template <template <class...> class R, std::size_t... Is, class T, class U>
constexpr auto combine(T&& t, U&& u, std::index_sequence<Is...>) noexcept {
  return R{std::forward<T>(t).template parser<Is>()..., std::forward<U>(u)};
}

template <template <class...> class R, std::size_t... Is, class T, class U>
constexpr auto combine_r(T&& t, U&& u, std::index_sequence<Is...>) noexcept {
  return R{std::forward<T>(t), std::forward<U>(u).template parser<Is>()...};
}

template <class T>
constexpr static inline auto make_index_sequence =
    dpsg::feed_t<std::decay_t<T>, std::index_sequence_for>{};

}  // namespace detail

template <class A,
          class B,
          detail::without_alternative<A> = 0,
          detail::without_alternative<B> = 0>
[[nodiscard]] constexpr auto operator|([[maybe_unused]] A&& left,
                                       [[maybe_unused]] B&& right) noexcept {
  return description::alternative{std::forward<A>(left),
                                  std::forward<B>(right)};
}

template <class T,
          class U,
          detail::with_sequence<T> = 0,
          detail::without_sequence<U> = 0>
[[nodiscard]] constexpr auto operator&(T&& t, U&& u) noexcept {
  return detail::combine<description::sequence>(
      std::forward<T>(t), std::forward<U>(u), detail::make_index_sequence<T>);
}

template <class T,
          class U,
          detail::with_sequence<U> = 0,
          detail::without_sequence<T> = 0>
[[nodiscard]] constexpr auto operator&(T&& t, U&& u) noexcept {
  return detail::combine_r<description::sequence>(
      std::forward<T>(t), std::forward<U>(u), detail::make_index_sequence<U>);
}

template <class T,
          class U,
          detail::with_alternative<T> = 0,
          detail::without_alternative<U> = 0>
[[nodiscard]] constexpr auto operator|(T&& t, U&& u) noexcept {
  return detail::combine<description::alternative>(
      std::forward<T>(t), std::forward<U>(u), detail::make_index_sequence<T>);
}

template <class T,
          class U,
          detail::with_alternative<U> = 0,
          detail::without_alternative<T> = 0>
[[nodiscard]] constexpr auto operator|(T&& t, U&& u) noexcept {
  return detail::combine_r<description::alternative>(
      std::forward<T>(t), std::forward<U>(u), detail::make_index_sequence<U>);
}

template <class A,
          class B,
          detail::without_sequence<A> = 0,
          detail::without_sequence<B> = 0>
[[nodiscard]] constexpr auto operator&([[maybe_unused]] A&& left,
                                       [[maybe_unused]] B&& right) noexcept {
  return description::both{std::forward<A>(left), std::forward<B>(right)};
}

template <class A,
          class B,
          detail::with_sequence<A> = 0,
          detail::with_sequence<B> = 0>
[[nodiscard]] constexpr auto operator&([[maybe_unused]] A&& left,
                                       [[maybe_unused]] B&& right) noexcept {
  return detail::combine<description::sequence>(std::forward<A>(left),
                                                std::forward<B>(right),
                                                detail::make_index_sequence<A>,
                                                detail::make_index_sequence<B>);
}

template <class A,
          class B,
          detail::with_alternative<A> = 0,
          detail::with_alternative<B> = 0>
[[nodiscard]] constexpr auto operator|([[maybe_unused]] A&& left,
                                       [[maybe_unused]] B&& right) noexcept {
  return detail::combine<description::alternative>(
      std::forward<A>(left),
      std::forward<B>(right),
      detail::make_index_sequence<A>,
      detail::make_index_sequence<B>);
}

constexpr description::dynamic_character<char> operator""_c(char c) noexcept {
  return description::dynamic_character<char>(c);
}

constexpr description::ascii::dynamic_case_insensitive_character<char>
operator""_ic(char c) noexcept {
  return description::ascii::dynamic_case_insensitive_character<char>(c);
}

constexpr description::static_string<char> operator""_s(
    const char* str,
    std::size_t size) noexcept {
  return description::static_string<char>{str, str + size};
}

constexpr description::ascii::case_insensitive_static_string<char>
operator""_is(const char* str, std::size_t size) noexcept {
  return description::ascii::case_insensitive_static_string<char>{str,
                                                                  str + size};
}

constexpr static inline description::end_t end{};
constexpr static inline description::any_t any{};
constexpr static inline description::succeed_t succeed{};

namespace detail {
struct is_zero : description::satisfy_character<is_zero> {
  template <class C>
  [[nodiscard]] constexpr bool operator()(C c) const noexcept {
    return c == static_cast<C>(0);
  }
};
}  // namespace detail

using eos_t = description::either<description::end_t, detail::is_zero>;
constexpr static inline eos_t eos{};

template <class T = void>
struct fail_t : description::fail_t<fail_t<T>> {
  using base = description::fail_t<fail_t<T>>;
  using base::operator();
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
  using base = description::fail_t<fail_t<void>>;
  using base::operator();

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