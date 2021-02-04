#ifndef GUARD_PARSERS_DESCRIPTION_RECURSIVE_HPP

#include "../utility.hpp"
#include "./containers.hpp"

namespace parsers::description {

struct self_t {};

namespace detail {
template <class T>
struct recursive_container {
  [[nodiscard]] constexpr auto parser() const noexcept {
    return typename T::parser_t{};
  }
};
}  // namespace detail

template <class T>
struct recursive : detail::recursive_container<recursive<T>> {
  constexpr friend std::true_type is_recursive_f(
      [[maybe_unused]] recursive _) noexcept;

  using parser_t = T;
};

namespace detail {
template <class R, class T>
struct replace_self {
  using type = T;
};
template <class R>
struct replace_self<R, ::parsers::description::self_t> {
  using type = R;
};

template <class R, template <class...> class C, class... Args>
struct replace_self<R, C<Args...>> {
  using type = C<typename replace_self<R, Args>::type...>;
};

template <class R, class T>
using replace_self_t = typename replace_self<R, T>::type;

template <class T, class = void>
struct has_plain_parser : std::false_type {};
template <class T>
struct has_plain_parser<
    T,
    std::enable_if_t<!std::is_same_v<decltype(std::declval<T>().parser()), T>>>
    : std::true_type {};

template <class T, class = void>
struct is_sequence : std::false_type {};
template <class T>
struct is_sequence<T, std::void_t<decltype(T::sequence_length)>>
    : std::true_type {};

template <class U, class T, std::size_t... Is>
constexpr replace_self_t<std::decay_t<U>, std::decay_t<T>> replace(
    T&& to_replace,
    U&& replacement,
    [[maybe_unused]] std::index_sequence<Is...>) noexcept {
  return replace_self_t<std::decay_t<U>, std::decay_t<T>>{
      replace(std::forward<T>(to_replace).template parser<Is>(),
              std::forward<U>(replacement))...};
}

template <class U, class T>
constexpr replace_self_t<std::decay_t<U>, std::decay_t<T>> replace(
    T&& to_replace,
    U&& replacement) noexcept {
  if constexpr (has_plain_parser<std::decay_t<T>>::value) {
    return replace(std::forward<T>(to_replace).parser(),
                   std::forward<U>(replacement));
  }
  else if constexpr (is_sequence<std::decay_t<T>>::value) {
    return replace(
        std::forward<T>(to_replace),
        std::forward<U>(replacement),
        std::make_index_sequence<std::decay_t<T>::sequence_length>{});
  }
  else if constexpr (std::is_same_v<std::decay_t<T>, self_t>) {
    return replacement;
  }
  else {
    return to_replace;
  }
}
}  // namespace detail

template <class T>
struct fix {
 private:
  using fixed_parser_t = detail::replace_self_t<fix, T>;
  using unfixed_parser_t = T;

  unfixed_parser_t _parser;

 public:
  using parser_t = fixed_parser_t;

  template <
      class U,
      std::enable_if_t<std::is_convertible_v<U, unfixed_parser_t>, int> = 0>
  constexpr explicit fix(U&& u) : _parser{std::forward<U>(u)} {}

  friend constexpr std::true_type is_recursive_f(fix) noexcept;

  constexpr parser_t parser() const noexcept {
    return detail::replace(_parser, *this);
  }
};
template <class T>
fix(T&&) -> fix<detail::remove_cvref_t<T>>;

constexpr std::false_type is_recursive_f(...) noexcept;
template <class T>
using is_recursive = decltype(is_recursive_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_recursive_v = is_recursive<T>::value;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_RECURSIVE_HPP