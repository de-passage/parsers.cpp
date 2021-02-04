#ifndef GUARD_PARSERS_DESCRIPTION_SATISFY_HPP
#define GUARD_PARSERS_DESCRIPTION_SATISFY_HPP

#include "../utility.hpp"

namespace parsers::description {

namespace detail {
template <class... Ts>
struct dynamic;
}

template <class CRTP>
struct satisfy {
  template <class... Args>
  [[nodiscard]] inline constexpr auto check(Args&&... args) noexcept {
    return static_cast<CRTP*>(this)->operator()(std::forward<Args>(args)...);
  }

  template <class... Args>
  [[nodiscard]] inline constexpr auto check(Args&&... args) const noexcept {
    return static_cast<const CRTP*>(this)->operator()(
        std::forward<Args>(args)...);
  }

  friend constexpr std::true_type is_satisfy_f(
      [[maybe_unused]] const satisfy& unused) noexcept {
    return {};
  }
};
template <class... Ts>
struct satisfy<detail::dynamic<Ts...>>
    : satisfy<satisfy<detail::dynamic<Ts...>>>, private Ts... {
  template <
      class... Us,
      std::enable_if_t<std::conjunction_v<std::is_convertible_v<Us, Ts>...>,
                       int> = 0>
  constexpr explicit satisfy(Us&&... us) : Ts{std::forward<Us>(us)}... {}

  using Ts::operator()...;
};

template <class U, class... Us>
satisfy(U&&, Us&&...)
    -> satisfy<detail::dynamic<std::decay_t<U>, std::decay_t<Us>...>>;

constexpr std::false_type is_satisfy_f(...) noexcept;

template <class T>
using is_satisfy = decltype(is_satisfy_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_satisfiable_predicate_v = is_satisfy<T>::value;

template <class T>
struct satisfy_character : satisfy<satisfy_character<T>> {
  template <class U, class V>
  [[nodiscard]] constexpr auto operator()(U&& begin, [[maybe_unused]] V&& end)
      const noexcept {
    auto b = begin;
    if (begin != end &&
        static_cast<const T*>(this)->operator()(*std::forward<U>(begin))) {
      return ++b;
    }
    return b;
  }
};

template <auto Char, class CharT = decltype(Char)>
struct character : satisfy_character<character<Char, CharT>> {
  constexpr static inline auto value = Char;
  using value_type = CharT;

  template <class C>
  [[nodiscard]] constexpr bool operator()(C&& c) const noexcept {
    return c == value;
  }
};

template <typename T>
struct character<nullptr, detail::dynamic<T>>
    : satisfy_character<character<nullptr, detail::dynamic<T>>> {
  using value_type = T;

  template <class C,
            std::enable_if_t<
                std::conjunction_v<
                    std::is_convertible<C, value_type>,
                    std::negation<std::is_same<std::decay_t<C>, character>>>,
                int> = 0>
  constexpr character(C&& c) noexcept : _value{std::forward<C>(c)} {}

  constexpr character(const character&) noexcept = default;
  constexpr character(character&&) noexcept = default;

  template <class C>
  [[nodiscard]] constexpr bool operator()(C&& c) const noexcept {
    return _value == c;
  }

  constexpr const value_type& value() const { return _value; }

 private:
  value_type _value;
};
template <class T>
using dynamic_character = character<nullptr, detail::dynamic<T>>;
template <class T>
character(T&&) -> character<nullptr, detail::dynamic<std::decay_t<T>>>;

struct any_t : satisfy_character<any_t> {
  template <class T>
  [[nodiscard]] constexpr bool operator()([[maybe_unused]] T&&) const noexcept {
    return true;
  }
};

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_SATISFY_HPP