#ifndef PARSERS_CUSTOMIZATION_POINTS_HPP
#define PARSERS_CUSTOMIZATION_POINTS_HPP

#include "../description.hpp"
#include "../utility.hpp"

namespace parsers::customization_points {
namespace detail {
using namespace ::parsers::detail;

template <class I, class R, class T>
using result_t = typename std::decay_t<I>::template result_t<std::decay_t<R>,
                                                             std::decay_t<T>>;

template <class I, class T, class... Args>
[[nodiscard]] constexpr auto success(Args&&... args) noexcept {
  return std::decay_t<I>::success(type<std::decay_t<T>>,
                                  std::forward<Args>(args)...);
}

template <class I, class T, class... Args>
[[nodiscard]] constexpr auto failure(Args&&... args) noexcept {
  return std::decay_t<I>::failure(type<std::decay_t<T>>,
                                  std::forward<Args>(args)...);
}

template <class I, class T, class... Args>
[[nodiscard]] constexpr auto combine(Args&&... args) noexcept {
  return std::decay_t<I>::combine(type<std::decay_t<T>>,
                                  std::forward<Args>(args)...);
}

template <class I, class T, class U, class V>
[[nodiscard]] constexpr auto init(U&& beg, V&& end) noexcept {
  return std::decay_t<I>::init(
      type<std::decay_t<T>>, std::forward<U>(beg), std::forward<V>(end));
}

template <class I, class R>
[[nodiscard]] constexpr auto next_iterator(R&& result) noexcept {
  return std::decay_t<I>::next_iterator(std::forward<R>(result));
}

template <class I, class R>
[[nodiscard]] constexpr auto has_value(R&& result) noexcept {
  return std::decay_t<I>::has_value(std::forward<R>(result));
}

template <class I, class T, class... Args>
[[nodiscard]] constexpr auto sequence(Args&&... args) noexcept {
  return std::decay_t<I>::sequence(type<std::decay_t<T>>,
                                   std::forward<Args>(args)...);
}
template <class I, class T, std::size_t S, class... Args>
[[nodiscard]] constexpr auto alternative(Args&&... args) noexcept {
  return std::decay_t<I>::template alternative<S>(type<std::decay_t<T>>,
                                                  std::forward<Args>(args)...);
}

template <class D, class I>
struct parser_indirection_t {
  using parser_t = D;
  using interpreter_t = I;

  parser_t parser;
  interpreter_t interpreter;

  template <class P, class J>
  constexpr explicit parser_indirection_t(P&& parser, J&& interpreter) noexcept
      : parser{std::forward<P>(parser)},
        interpreter{std::forward<J>(interpreter)} {
    static_assert(std::is_same_v<std::decay_t<P>, parser_t>);
    static_assert(std::is_same_v<std::decay_t<J>, interpreter_t>);
  }

  template <class K, class J>
  [[nodiscard]] constexpr inline auto operator()(K begin, J end) const noexcept
      -> result_t<I, K, D> {
    if constexpr (std::is_invocable_v<decltype(interpreter),
                                      decltype(parser),
                                      decltype(interpreter)>) {
      return interpreter(parser, interpreter)(begin, end);
    }
    else {
      return interpreter(parser)(begin, end);
    }
  }
};

template <class G, class I>
struct guard_parser {
  G guard;
  template <class T, class U>
  constexpr auto operator()(T beg, U end) const noexcept
      -> detail::result_t<I, T, G> {
    if (guard(beg, end)) {
      return detail::success<I, G>(beg, beg, end);
    }
    return detail::failure<I, G>(beg, beg, end);
  }
};

template <class T, class I>
struct predicate_parser {
  T pred;
  template <class U, class V>
  constexpr auto operator()(U beg, V end) const noexcept
      -> detail::result_t<I, U, T> {
    auto r = pred.check(beg, end);
    if (r != beg) {
      return detail::success<I, T>(beg, r, end);
    }
    return detail::failure<I, T>(beg, beg, end);
  };
};

template <class M, class I, class P>
struct dynamic_range_parser {
  std::size_t expected;
  P parser;
  template <class T, class U>
  constexpr auto operator()(T beg, U end) const noexcept
      -> detail::result_t<I, decltype(beg), M> {
    auto acc = detail::success<I, M>(beg, beg, end);
    std::size_t count = 0;
    auto b = beg;
    while (beg != end) {
      auto r = detail::combine<I, M>(acc, parser(beg, end));
      if (!detail::has_value<I>(r)) {
        if (count >= expected) {
          break;
        }
        return detail::failure<I, M>(b, beg, end);
      }
      beg = detail::next_iterator<I>(std::move(r));
      ++count;
    }
    return acc;
  }
};

template <std::size_t S, class D, class I, class ItB, class ItE, class... Args>
constexpr detail::result_t<I, ItB, D> call_sequence(
    [[maybe_unused]] D&& descriptor,
    I&& interpreter,
    [[maybe_unused]] ItB beg,
    [[maybe_unused]] ItB cur,
    [[maybe_unused]] ItE end,
    Args&&... args) noexcept {
  if constexpr (S < std::decay_t<D>::sequence_length) {
    auto r = std::forward<I>(interpreter)(
        std::forward<D>(descriptor).template parser<S>())(cur, end);
    if (detail::has_value<I>(r)) {
      return call_sequence<S + 1>(std::forward<D>(descriptor),
                                  std::forward<I>(interpreter),
                                  beg,
                                  detail::next_iterator<I>(r),
                                  end,
                                  std::forward<Args>(args)...,
                                  std::move(r));
    }
    return detail::failure<I, D>(beg, cur, end);
  }
  else {
    return detail::sequence<I, D>(std::forward<Args>(args)...);
  }
}

template <class S, class I>
struct sequence_parser {
  S descriptor;
  I interpreter;

  template <class T, class U>
  constexpr auto operator()(T beg, U e) const noexcept
      -> detail::result_t<I, T, S> {
    return detail::call_sequence<0>(
        std::move(descriptor), std::move(interpreter), beg, beg, e);
  }
};

template <std::size_t S, class D, class I, class ItB, class ItE>
constexpr detail::result_t<I, ItB, D> call_alternative(
    [[maybe_unused]] D&& descriptor,
    I&& interpreter,
    [[maybe_unused]] ItB beg,
    [[maybe_unused]] ItE end) noexcept {
  if constexpr (S < std::decay_t<D>::sequence_length) {
    auto r = std::forward<I>(interpreter)(
        std::forward<D>(descriptor).template parser<S>())(beg, end);
    if (detail::has_value<I>(r)) {
      return detail::alternative<I, D, S>(std::move(r));
    }
    return call_alternative<S + 1>(
        std::forward<D>(descriptor), std::forward<I>(interpreter), beg, end);
  }
  else {
    return detail::failure<I, D>(beg, beg, end);
  }
}

template <class A, class I>
struct alternative_parser {
  A descriptor;
  I interpreter;

  template <class T, class U>
  constexpr auto operator()(T beg, U e) const noexcept
      -> detail::result_t<I, T, A> {
    return detail::call_alternative<0>(
        std::move(descriptor), std::move(interpreter), beg, e);
  }
};

template <class T, class P, class A, class B, class = void>
struct has_modify : std::false_type {};
template <class T, class P, class A, class B>
struct has_modify<T,
                  P,
                  A,
                  B,
                  std::void_t<decltype(T::modify(
                      std::declval<P>()(std::declval<A>(), std::declval<B>()),
                      std::declval<A>(),
                      std::declval<B>()))>> : std::true_type {};

template <class P, class I, class D>
struct modifier_parser {
  P parser;
  template <class ItB, class ItE>
  constexpr auto operator()(ItB begin, ItE end) const noexcept
      -> detail::result_t<I, ItB, D> {
    if constexpr (has_modify<I, P, ItB, ItE>::value) {
      return I::modify(parser(begin, end), begin, end);
    }
    else {
      return parser(begin, end);
    }
  }
};

}  // namespace detail

template <class T,
          class I,
          std::enable_if_t<description::is_satisfiable_predicate_v<T>, int> = 0>
constexpr auto parsers_interpreters_make_parser(T&& pred,
                                                [[maybe_unused]] I&&) {
  return detail::predicate_parser<T, I>{std::forward<T>(pred)};
}

template <class T,
          class I,
          std::enable_if_t<description::is_guard_v<T>, int> = 0>
constexpr auto parsers_interpreters_make_parser(T&& pred,
                                                [[maybe_unused]] I&&) {
  return detail::guard_parser<T, I>{std::forward<T>(pred)};
}

template <
    class M,
    class I,
    std::enable_if_t<description::is_dynamic_range_v<std::decay_t<M>>, int> = 0>
constexpr auto parsers_interpreters_make_parser(M&& descriptor,
                                                I&& interpreter) noexcept {
  return detail::dynamic_range_parser<
      std::decay_t<M>,
      std::decay_t<I>,
      decltype(interpreter(std::forward<M>(descriptor).parser()))>{
      descriptor.count(), interpreter(std::forward<M>(descriptor).parser())};
}

template <class R,
          class I,
          std::enable_if_t<description::is_recursive_v<R>, int> = 0>
constexpr auto parsers_interpreters_make_parser(R&& descriptor,
                                                I&& interpreter) noexcept {
  return detail::parser_indirection_t<typename std::decay_t<R>::parser_t,
                                      std::decay_t<I>>{
      std::forward<R>(descriptor).parser(), std::forward<I>(interpreter)};
}

template <class S,
          class I,
          std::enable_if_t<description::is_sequence_v<S>, int> = 0>
constexpr auto parsers_interpreters_make_parser(
    S&& descriptor,
    [[maybe_unused]] I&& interpreter) noexcept {
  return detail::sequence_parser<std::decay_t<S>, std::decay_t<I>>{
      std::forward<S>(descriptor), std::forward<I>(interpreter)};
}

template <class A,
          class I,
          std::enable_if_t<description::is_alternative_v<A>, int> = 0>
constexpr auto parsers_interpreters_make_parser(
    A&& descriptor,
    [[maybe_unused]] I&& interpreter) noexcept {
  return detail::alternative_parser<std::decay_t<A>, std::decay_t<I>>{
      std::forward<A>(descriptor), std::forward<I>(interpreter)};
}
template <
    class D,
    class J,
    std::enable_if_t<description::is_modifier_v<std::decay_t<D>>, int> = 0>
constexpr auto parsers_interpreters_make_parser(
    D&& descriptor,
    [[maybe_unused]] J&& ignore) noexcept {
  return detail::modifier_parser<decltype(descriptor.get_p()),
                                 std::decay_t<J>,
                                 std::decay_t<D>>{
      std::forward<D>(descriptor).get_p()};
}

}  // namespace parsers::customization_points

#endif  // PARSERS_CUSTOMIZATION_POINTS_HPP