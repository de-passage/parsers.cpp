#ifndef GUARD_PARSERS_DESCRIPTIONS_HPP
#define GUARD_PARSERS_DESCRIPTIONS_HPP

#include "./utility.hpp"

#include <optional>
#include <type_traits>

namespace parsers::description {
namespace detail {
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
}

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

template <auto Char, class CharT = decltype(Char)>
struct character : satisfy<character<Char, CharT>> {
  constexpr static inline auto value = Char;
  using value_type = CharT;

  template <class C>
  [[nodiscard]] constexpr bool operator()(C&& c) const noexcept {
    return c == value;
  }
};

namespace detail {
template <class T>
struct dynamic;
}

template <typename T>
struct character<nullptr, detail::dynamic<T>>
    : satisfy<character<nullptr, detail::dynamic<T>>> {
  using value_type = T;

  template <class C,
            std::enable_if_t<
                std::negation_v<std::is_same<std::decay_t<C>, character>>,
                int> = 0>
  constexpr character(C&& c) noexcept : _value{std::forward<C>(c)} {}

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

struct any_t : satisfy<any_t> {
  [[nodiscard]] constexpr bool operator()(...) const noexcept { return true; }
};

template <class T>
struct fail_t {
  friend constexpr std::true_type is_failure_f(fail_t) noexcept;
};
constexpr std::false_type is_failure_f(...) noexcept;
template <class T>
using is_failure = decltype(is_failure_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_failure_v = is_failure<T>::value;

struct succeed_t {};

template <class A, class B>
struct empty_pair {
  using left_t = A;
  using right_t = B;
  constexpr static inline left_t left() noexcept { return {}; }
  constexpr static inline right_t right() noexcept { return {}; }
};

namespace detail {
template <class It, class Out>
constexpr void copy(It beg, It end, Out out) noexcept {
  while (beg != end) {
    *out++ = *beg++;
  }
}
}  // namespace detail

template <class T>
struct container {
  using parser_t = T;

  constexpr container() noexcept = default;
  template <class U,
            std::enable_if_t<!std::is_array_v<std::remove_reference_t<U>> &&
                                 !std::is_same_v<std::decay_t<U>, container>,
                             int> = 0>
  constexpr explicit container(U&& t) noexcept : _parser{std::forward<U>(t)} {}
  template <
      class U,
      std::enable_if_t<std::is_array_v<std::remove_reference_t<U>>, int> = 0>
  constexpr explicit container(U&& t) noexcept
  // clang-format off
#if defined(_MSC_VER)
      : _parser{}
#endif
  { 
    detail::copy(std::begin(t), std::end(t), std::begin(_parser)); 
  }
  // clang-format on

  constexpr container(container&&) noexcept = default;
  constexpr container(const container&) noexcept = default;

  [[nodiscard]] constexpr parser_t& parser() & noexcept { return _parser; }
  [[nodiscard]] constexpr const parser_t& parser() const& noexcept {
    return _parser;
  }
  [[nodiscard]] constexpr parser_t&& parser() && noexcept {
    return static_cast<container&&>(*this)._parser;
  }
  [[nodiscard]] constexpr const parser_t&& parser() const&& noexcept {
    return static_cast<const container&&>(*this)._parser;
  }

 private:
  parser_t _parser;
};

template <class T>
struct empty_container {
  using parser_t = T;
  [[nodiscard]] constexpr static inline parser_t parser() noexcept {
    return parser_t{};
  }
};

namespace detail {
template <class T>
struct requires_initialization : std::true_type {};
template <class T>
struct requires_initialization<empty_container<T>> : std::false_type {};
template <class T>
constexpr static inline bool requires_initialization_v =
    requires_initialization<T>::value;

template <std::size_t S, class T, class... Args>
struct at {
  using type = typename at<S - 1, Args...>::type;
};
template <class T, class... Args>
struct at<0, T, Args...> {
  using type = T;
};
template <std::size_t S, class... Args>
using at_t = typename at<S, Args...>::type;

template <std::size_t S, class T>
struct indexed_container : private container<T> {
  using base = container<T>;

  constexpr indexed_container() noexcept = default;

  template <
      class U,
      std::enable_if_t<
          std::conjunction_v<
              std::is_constructible<base, U>,
              std::negation<std::is_same<std::decay_t<U>, indexed_container>>>,
          int> = 0>
  constexpr explicit indexed_container(U&& u) : base{std::forward<U>(u)} {}

  constexpr decltype(auto) parser() const& noexcept { return base::parser(); }

  constexpr decltype(auto) parser() & noexcept { return base::parser(); }

  constexpr decltype(auto) parser() && noexcept {
    return static_cast<indexed_container&&>(*this).base::parser();
  }

  constexpr decltype(auto) parser() const&& noexcept {
    return static_cast<const indexed_container&&>(*this).base::parser();
  }

  using parser_t = typename base::parser_t;
};

template <class T, class... Ss>
struct indexed_sequence;
template <std::size_t... Ss, class... Ts>
struct indexed_sequence<std::index_sequence<Ss...>, Ts...>
    : indexed_container<Ss, Ts>... {
  constexpr explicit indexed_sequence() = default;
  template <class... Us>
  constexpr explicit indexed_sequence(Us&&... us)
      : indexed_container<Ss, Ts>{std::forward<Us>(us)}... {}

  template <std::size_t S>
  constexpr decltype(auto) parser() const& noexcept {
    return indexed_container<S, at_t<S, Ts...>>::parser();
  }

  template <std::size_t S>
  constexpr decltype(auto) parser() & noexcept {
    return indexed_container<S, at_t<S, Ts...>>::parser();
  }

  template <std::size_t S>
  constexpr decltype(auto) parser() const&& noexcept {
    return static_cast<const indexed_sequence&&>(*this)
        .indexed_container<S, at_t<S, Ts...>>::parser();
  }

  template <std::size_t S>
  constexpr decltype(auto) parser() && noexcept {
    return static_cast<indexed_sequence&&>(*this)
        .indexed_container<S, at_t<S, Ts...>>::parser();
  }

  template <std::size_t S>
  using parser_t = typename indexed_container<S, at_t<S, Ts...>>::parser_t;

  constexpr static inline std::size_t sequence_length = sizeof...(Ss);
};

}  // namespace detail

template <class A, class B>
struct pair : detail::indexed_sequence<std::index_sequence<0, 1>, A, B> {
  using base = detail::indexed_sequence<std::index_sequence<0, 1>, A, B>;

 public:
  using left_t = typename base::template parser_t<0>;
  using right_t = typename base::template parser_t<1>;

  constexpr pair() = default;

  template <class T, class U>
  constexpr pair(T&& l, U&& r) noexcept
      : base{std::forward<T>(l), std::forward<U>(r)} {}
  constexpr inline decltype(auto) left() const& noexcept {
    return base::template parser<0>();
  }
  constexpr inline decltype(auto) right() const& noexcept {
    return base::template parser<1>();
  }
  constexpr inline decltype(auto) left() & noexcept {
    return base::template parser<0>();
  }
  constexpr inline decltype(auto) right() & noexcept {
    return base::template parser<1>();
  }
  constexpr inline decltype(auto) left() const&& noexcept {
    return static_cast<const pair&&>(*this).base::template parser<0>();
  }
  constexpr inline decltype(auto) right() const&& noexcept {
    return static_cast<const pair&&>(*this).base::template parser<1>();
  }
  constexpr inline decltype(auto) left() && noexcept {
    return static_cast<pair&&>(*this).base::template parser<0>();
  }
  constexpr inline decltype(auto) right() && noexcept {
    return static_cast<pair&&>(*this).base::template parser<1>();
  }
};

template <class A, class B, class C = pair<A, B>>
struct either : C {
  constexpr either() = default;
  template <class T, class U>
  constexpr either(T&& t, U&& u) noexcept
      : C{std::forward<T>(t), std::forward<U>(u)} {}
};
template <class A,
          class B,
          class A1 = detail::remove_cvref_t<A>,
          class B1 = detail::remove_cvref_t<B>>
either(A&&, B&&) -> either<A1, B1, pair<A1, B1>>;

template <class A, class B, class C = pair<A, B>>
struct both : C {
  constexpr both() = default;
  template <class T, class U>
  constexpr both(T&& t, U&& u) noexcept
      : C{std::forward<T>(t), std::forward<U>(u)} {}
};

template <class A,
          class B,
          class A1 = detail::remove_cvref_t<A>,
          class B1 = detail::remove_cvref_t<B>>
both(A&&, B&&) -> both<A1, B1, pair<A1, B1>>;

template <class P, class C = empty_container<P>>
struct many : C {
  constexpr many() noexcept = default;
  template <class Q>
  constexpr many(Q&& q) : C{std::forward<Q>(q)} {}
};
template <class P, class P1 = detail::remove_cvref_t<P>>
many(P&&) -> many<P1, container<P1>>;

struct end_t {};

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

template <class T, class F>
struct stateful_recursive_container {
 private:
  using unfixed_parser_t = T;

  template <class U>
  struct pointer_container {
    using parser_t = U;
    template <class P,
              std::enable_if_t<std::is_same_v<std::decay_t<P>, U>, int> = 0>
    constexpr pointer_container(P&& p) : U{std::forward<P>(p)} {}

    constexpr const parser_t& parser() const noexcept { return *this; }
  };

 public:
  using parser_t = pointer_container<
      detail::replace_self_t<pointer_container<unfixed_parser_t>,
                             unfixed_parser_t>>;

 private:
  parser_t _parser;

 public:
  template <
      class U,
      std::enable_if_t<std::is_convertible_v<unfixed_parser_t, U>, int> = 0>
  constexpr stateful_recursive_container(U&& p) noexcept
      : _parser{std::forward<U>(p)} {}

  constexpr const parser_t& parser() const noexcept { return _parser; }

  friend constexpr std::true_type is_recursive_f(
      stateful_recursive_container) noexcept;
};

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
struct fixed {
 private:
  using fixed_parser_t = detail::replace_self_t<fixed, T>;
  using unfixed_parser_t = T;

  unfixed_parser_t _parser;

 public:
  using parser_t = fixed_parser_t;

  template <
      class U,
      std::enable_if_t<std::is_convertible_v<unfixed_parser_t, std::decay_t<U>>,
                       int> = 0>
  constexpr explicit fixed(U&& u) : _parser{std::forward<U>(u)} {}

  friend constexpr std::true_type is_recursive_f(fixed) noexcept;

  constexpr parser_t parser() const noexcept {
    return detail::replace(_parser, *this);
  }
};

constexpr std::false_type is_recursive_f(...) noexcept;
template <class T>
using is_recursive = decltype(is_recursive_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_recursive_v = is_recursive<T>::value;

template <class Char>
struct static_string {
  using char_t = Char;
  using pointer_t = const Char*;
  constexpr static_string(pointer_t beg, pointer_t end) noexcept
      : _begin(beg), _end(end) {}

  template <class T, std::size_t S>
  constexpr explicit static_string(T (&arr)[S]) noexcept
      : _begin(static_cast<pointer_t>(arr)),
        _end(static_cast<pointer_t>(arr) + S - 1) {}

  constexpr auto begin() const noexcept { return _begin; }
  constexpr auto cbegin() const noexcept { return _begin; }
  constexpr auto end() const noexcept { return _end; }
  constexpr auto cend() const noexcept { return _end; }

 private:
  pointer_t _begin;
  pointer_t _end;
};
template <class T, std::size_t S>
static_string(T (&)[S]) -> static_string<std::decay_t<T>>;

template <class... Ss>
struct sequence
    : detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...> {
  using base = detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...>;

  static_assert(base::sequence_length > 0, "Empty sequence not allowed");
  constexpr sequence() noexcept = default;
  template <class... Ts,
            std::enable_if_t<std::conjunction_v<std::is_convertible<Ts, Ss>...>,
                             int> = 0>
  constexpr explicit sequence(Ts&&... ts) noexcept
      : base{std::forward<Ts>(ts)...} {}
};
template <class A, class... Args>
sequence(A&&, Args&&...) -> sequence<std::decay_t<A>, std::decay_t<Args>...>;

template <class... Ss>
struct alternative
    : detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...> {
  using base = detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...>;

  static_assert(base::sequence_length > 0, "Empty alternative not allowed");
  constexpr alternative() noexcept = default;
  template <class... Ts,
            std::enable_if_t<std::conjunction_v<std::is_convertible<Ts, Ss>...>,
                             int> = 0>
  constexpr explicit alternative(Ts&&... ts) noexcept
      : base{std::forward<Ts>(ts)...} {}
};
template <class A, class... Args>
alternative(A&&, Args&&...)
    -> alternative<std::decay_t<A>, std::decay_t<Args>...>;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTIONS_HPP