#ifndef GUARD_PARSERS_DESCRIPTIONS_HPP
#define GUARD_PARSERS_DESCRIPTIONS_HPP

#include "./utility.hpp"

#include <optional>
#include <type_traits>

namespace parsers::description {
namespace detail {
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <class... Ts>
struct dynamic;
}  // namespace detail

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

template <typename T>
struct character<nullptr, detail::dynamic<T>>
    : satisfy<character<nullptr, detail::dynamic<T>>> {
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
                                 !std::is_same_v<std::decay_t<U>, container> &&
                                 std::is_convertible_v<U, parser_t>,
                             int> = 0>
  constexpr explicit container(U&& t) noexcept : _parser{std::forward<U>(t)} {}

  template <class U,
            std::enable_if_t<std::is_array_v<std::remove_reference_t<U>> &&
                                 !std::is_same_v<std::decay_t<U>, container>,
                             int> = 0>
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
  constexpr container(const container& c) noexcept = default;

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
struct indexed_container : container<T> {
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

template <class... Args>
using make_indexed_sequence =
    indexed_sequence<std::index_sequence_for<Args...>, Args...>;

}  // namespace detail

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

  friend constexpr std::true_type is_sequence_f(const sequence&) noexcept;
};
template <class A, class... Args>
sequence(A&&, Args&&...) -> sequence<std::decay_t<A>, std::decay_t<Args>...>;

constexpr std::false_type is_sequence_f(...) noexcept;

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

  friend constexpr std::true_type is_alternative_f(const alternative&) noexcept;
};
template <class A, class... Args>
alternative(A&&, Args&&...)
    -> alternative<std::decay_t<A>, std::decay_t<Args>...>;

constexpr std::false_type is_alternative_f(...) noexcept;

template <class T>
using is_alternative = decltype(is_alternative_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_alternative_v = is_alternative<T>::value;
template <class T>
using is_sequence = decltype(is_sequence_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_sequence_v = is_sequence<T>::value;

constexpr std::false_type is_dynamic_range_f(...) noexcept;
template <class T>
using is_dynamic_range = decltype(is_dynamic_range_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_dynamic_range_v = is_dynamic_range<T>::value;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTIONS_HPP