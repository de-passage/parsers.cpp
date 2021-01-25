#ifndef GUARD_PARSERS_DESCRIPTIONS_HPP
#define GUARD_PARSERS_DESCRIPTIONS_HPP

#include <type_traits>

namespace parsers::description {

template <class CRTP>
struct satisfy {
  template <class... Args>
  [[nodiscard]] inline constexpr bool operator()(Args&&... args) noexcept {
    return static_cast<CRTP*>(this)->operator()(std::forward<Args>(args)...);
  }

  template <class... Args>
  [[nodiscard]] inline constexpr bool operator()(
      Args&&... args) const noexcept {
    return static_cast<const CRTP*>(this)->operator()(
        std::forward<Args>(args)...);
  }

  friend constexpr std::true_type is_satisfiable_predicate_f(
      [[maybe_unused]] const satisfy& unused) noexcept {
    return {};
  }
};
constexpr std::false_type is_satisfiable_predicate_f(...) noexcept {
  return {};
}

template <class T>
using is_satisfiable_predicate =
    decltype(is_satisfiable_predicate_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_satisfiable_predicate_v =
    is_satisfiable_predicate<T>::value;

template <auto Char>
struct character : satisfy<character<Char>> {
  constexpr static inline auto value = Char;
  using value_type = decltype(Char);

  template <class C>
  [[nodiscard]] constexpr bool operator()(C&& c) const noexcept {
    return c == Char;
  }
};

struct any : satisfy<any> {
  [[nodiscard]] constexpr bool operator()(...) const noexcept { return true; }
};
struct fail {};
struct succeed {};

template <class A, class B>
struct pair {
  using left_t = A;
  using right_t = B;
  constexpr static inline left_t left() noexcept { return {}; }
  constexpr static inline right_t right() noexcept { return {}; }
};

template <class A, class B>
struct either : pair<A, B> {};

template <class A, class B>
[[nodiscard]] constexpr auto operator|([[maybe_unused]] A&& left,
                                       [[maybe_unused]] B&& right) noexcept {
  return either<std::decay_t<A>, std::decay_t<B>>{};
}

template <class A, class B>
struct both : pair<A, B> {};

template <class A, class B>
both(A&&, B&&) -> both<std::decay_t<A>, std::decay_t<B>>;
template <class A, class B>

[[nodiscard]] constexpr auto operator+([[maybe_unused]] A&& left,
                                       [[maybe_unused]] B&& right) noexcept {
  return both<std::decay_t<A>, std::decay_t<B>>{};
}

template <class P>
struct many {
  using parser_t = P;
  constexpr inline static parser_t parser() noexcept { return {}; }
};

struct end {};

namespace crtp {
template <class T>
struct construct_parser_t {
  [[nodiscard]] constexpr auto parser() noexcept {
    return typename T::parser_t{};
  }
};
}  // namespace crtp

template <class T>
struct recursive : crtp::construct_parser_t<recursive<T>> {
  constexpr friend std::true_type is_recursive_f(
      [[maybe_unused]] recursive _) noexcept;

  using parser_t = T;
};
constexpr std::false_type is_recursive_f(...) noexcept;
template <class T>
using is_recursive = decltype(is_recursive_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_recursive_v = is_recursive<T>::value;

template <class Char>
struct static_string {
  const Char* begin;
  const Char* end;
};

constexpr static_string<char> operator""_s(const char* str,
                                           unsigned long long size) noexcept {
  return static_string<char>{str, str + size};
}

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTIONS_HPP