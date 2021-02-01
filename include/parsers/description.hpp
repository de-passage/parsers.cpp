#ifndef GUARD_PARSERS_DESCRIPTIONS_HPP
#define GUARD_PARSERS_DESCRIPTIONS_HPP

#include "./utility.hpp"

#include <array>
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
  constexpr explicit container(U&& t) noexcept {
    detail::copy(std::begin(t), std::end(t), std::begin(_parser));
  }
  constexpr container(container&&) noexcept = default;
  constexpr container(const container&) noexcept = default;

  [[nodiscard]] constexpr parser_t& parser() & noexcept { return _parser; }
  [[nodiscard]] constexpr const parser_t& parser() const& noexcept {
    return _parser;
  }
  [[nodiscard]] constexpr parser_t&& parser() && noexcept {
    return static_cast<container&&>(this)._parser;
  }
  [[nodiscard]] constexpr const parser_t&& parser() const&& noexcept {
    return static_cast<const container&&>(this)._parser;
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

template <class A, class B>
struct pair {
 public:
  using left_t = A;
  using right_t = B;

 private:
  using left_container = container<left_t>;
  using right_container = container<right_t>;
  left_container _left;
  right_container _right;

 public:
  constexpr pair() = default;

  template <class T, class U>
  constexpr pair(T&& l, U&& r) noexcept
      : _left{std::forward<T>(l)}, _right{std::forward<U>(r)} {}
  constexpr inline const left_t& left() const& noexcept {
    return _left.parser();
  }
  constexpr inline const right_t& right() const& noexcept {
    return _right.parser();
  }
  constexpr inline left_t& left() & noexcept { return _left.parser(); }
  constexpr inline right_t& right() & noexcept { return _right.parser(); }
  constexpr inline const left_t&& left() const&& noexcept {
    return static_cast<const pair&&>(*this)._left.parser();
  }
  constexpr inline const right_t&& right() const&& noexcept {
    return static_cast<const pair&&>(*this)._right.parser();
  }
  constexpr inline left_t&& left() && noexcept {
    return static_cast<const pair&&>(*this)._left.parser();
  }
  constexpr inline right_t&& right() && noexcept {
    return static_cast<const pair&&>(*this)._right.parser();
  }
};

template <class A, class B, class C = empty_pair<A, B>>
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

template <class A, class B, class C = empty_pair<A, B>>
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

namespace detail {
template <class T>
struct construct_parser_t {
  [[nodiscard]] constexpr auto parser() noexcept {
    return typename T::parser_t{};
  }
};
}  // namespace detail

template <class T>
struct recursive : detail::construct_parser_t<recursive<T>> {
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
struct indexed_container
    : private std::conditional_t<requires_initialization_v<T>,
                                 container<T>,
                                 empty_container<T>> {
  using base = std::conditional_t<requires_initialization_v<T>,
                                  container<T>,
                                  empty_container<T>>;

  constexpr indexed_container() noexcept = default;

  template <
      class U,
      std::enable_if_t<
          std::conjunction_v<
              std::is_constructible<T, U>,
              std::negation<std::is_same<std::decay_t<U>, indexed_container>>>,
          int> = 0>
  constexpr explicit indexed_container(U&& u) : base{std::forward<U>(u)} {}

  template <std::size_t R, std::enable_if_t<S == R, int> = 0>
  constexpr auto parser() const noexcept {
    return base::parser();
  }
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
  constexpr auto parser() const noexcept {
    return indexed_container<S, at_t<S, Ts...>>::template parser<S>();
  }
};

}  // namespace detail

template <class... Ss>
struct sequence
    : detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...> {
  using base = detail::indexed_sequence<std::index_sequence_for<Ss...>, Ss...>;

  constexpr static inline std::size_t sequence_length = sizeof...(Ss);

  static_assert(sequence_length > 0, "Empty sequence not allowed");
  constexpr sequence() noexcept = default;
  template <
      class... Ts,
      std::enable_if_t<sequence_length == sizeof...(Ts) &&
                           std::conjunction_v<std::is_convertible<Ts, Ss>...>,
                       int> = 0>
  constexpr explicit sequence(Ts&&... ts) noexcept
      : base{std::forward<Ts>(ts)...} {}
};

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTIONS_HPP