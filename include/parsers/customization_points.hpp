#ifndef PARSERS_CUSTOMIZATION_POINTS_HPP
#define PARSERS_CUSTOMIZATION_POINTS_HPP

#include "./description.hpp"
#include "./result.hpp"
#include "./utility.hpp"

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

template <class I, class T, class R>
[[nodiscard]] constexpr auto left(R&& result) noexcept {
  return std::decay_t<I>::left(type<std::decay_t<T>>, std::forward<R>(result));
}

template <class I, class T, class R>
[[nodiscard]] constexpr auto right(R&& result) noexcept {
  return std::decay_t<I>::right(type<std::decay_t<T>>, std::forward<R>(result));
}

template <class I, class T, class L, class R>
[[nodiscard]] constexpr auto both(L&& left, R&& right) noexcept {
  return std::decay_t<I>::both(
      type<std::decay_t<T>>, std::forward<L>(left), std::forward<R>(right));
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

template <class F, class I>
struct fail_parser {
  template <class T, class U>
  constexpr auto operator()(T beg, U end) const noexcept
      -> detail::result_t<I, T, F> {
    return detail::failure<I, F>(beg, beg, end);
  }
};

template <class T, class I>
struct predicate_parser {
  T pred;
  template <class U, class V>
  constexpr auto operator()(U beg, V end) const noexcept
      -> detail::result_t<I, U, T> {
    if (beg != end && pred(*beg)) {
      auto before = beg;
      return detail::success<I, T>(before, ++beg, end);
    }
    return detail::failure<I, T>(beg, beg, end);
  };
};

template <class I>
struct end_parser {
  template <class U, class V>
  constexpr auto operator()(U beg, V end) const noexcept
      -> detail::result_t<I, U, description::end_t> {
    if (beg == end) {
      return detail::success<I, description::end_t>(beg, beg, end);
    }
    return detail::failure<I, description::end_t>(beg, beg, end);
  }
};

template <class M, class I, class P>
struct many_parser {
  P parser;
  template <class T, class U>
  constexpr auto operator()(T beg, U end) const noexcept
      -> detail::result_t<I, decltype(beg), M> {
    auto acc = detail::success<I, M>(beg, beg, end);
    while (beg != end) {
      auto r = detail::combine<I, M>(acc, parser(beg, end));
      if (!detail::has_value<I>(r)) {
        break;
      }
      beg = detail::next_iterator<I>(std::move(r));
    }
    return acc;
  }
};

template <class B, class I, class L, class R>
struct both_parser {
  L left;
  R right;

  template <class T, class U>
  constexpr auto operator()(T beg, U end) const noexcept
      -> detail::result_t<I, T, B> {
    if (auto r1 = left(beg, end); detail::has_value<I>(r1)) {
      if (auto r2 = right(detail::next_iterator<I>(r1), end);
          detail::has_value<I>(r2)) {
        return detail::sequence<I, B>(std::move(r1), std::move(r2));
      }
      return detail::failure<I, B>(beg, detail::next_iterator<I>(r1), end);
    }
    return detail::failure<I, B>(beg, beg, end);
  }
};

template <class E, class I, class L, class R>
struct either_parser {
  L left;
  R right;

  template <class T, class U>
  constexpr auto operator()(T beg, U end) const noexcept
      -> detail::result_t<I, T, E> {
    if (auto l = left(beg, end); detail::has_value<I>(l)) {
      return detail::alternative<I, E, 0>(std::move(l));
    }
    auto r = right(beg, end);
    if (detail::has_value<I>(r)) {
      return detail::alternative<I, E, 1>(std::move(r));
    }
    return detail::failure<I, E>(beg, beg, end);
  }
};

template <class S, class I>
struct static_string_parser {
  S descriptor;

  template <class T, class U>
  constexpr auto operator()(T beg, U e) const noexcept
      -> detail::result_t<I, T, S> {
    using std::begin;
    using std::end;
    auto itb = begin(descriptor);
    auto b = beg;
    while (itb != end(descriptor)) {
      if (beg == e || *beg != *itb) {
        return detail::failure<I, S>(b, beg, e);
      }
      ++beg;
      ++itb;
    }
    return detail::success<I, S>(b, beg, e);
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

template <class I>
struct succeed_parser {
  template <class T, class U>
  constexpr auto operator()(T beg, U end) const noexcept
      -> detail::result_t<I, T, description::succeed_t> {
    return detail::success<I, description::succeed_t>(beg, beg, end);
  };
};

}  // namespace detail

template <class F,
          class I,
          std::enable_if_t<description::is_failure_v<F>, int> = 0>
constexpr auto parsers_interpreters_make_parser([[maybe_unused]] F&&,
                                                [[maybe_unused]] I&&) noexcept {
  return detail::fail_parser<F, I>{};
}

template <class T,
          class I,
          std::enable_if_t<description::is_satisfiable_predicate_v<T>, int> = 0>
constexpr auto parsers_interpreters_make_parser(T&& pred,
                                                [[maybe_unused]] I&&) {
  return detail::predicate_parser<T, I>{std::forward<T>(pred)};
}

template <class I>
constexpr auto parsers_interpreters_make_parser(
    [[maybe_unused]] description::end_t,
    [[maybe_unused]] I&&) noexcept {
  return detail::end_parser<I>{};
}

template <class M, class I, detail::instance_of<M, description::many> = 0>
constexpr auto parsers_interpreters_make_parser(M&& descriptor,
                                                I interpreter) noexcept {
  return detail::many_parser<M, I, decltype(interpreter(descriptor.parser()))>{
      interpreter(descriptor.parser())};
}

template <class B, class I, detail::instance_of<B, description::both> = 0>
constexpr auto parsers_interpreters_make_parser(B&& descriptor,
                                                I interpreter) noexcept {
  return detail::both_parser<B,
                             I,
                             decltype(interpreter(descriptor.left())),
                             decltype(interpreter(descriptor.right()))>{
      interpreter(descriptor.left()), interpreter(descriptor.right())};
}

template <class E, class I, detail::instance_of<E, description::either> = 0>
constexpr auto parsers_interpreters_make_parser(E&& descriptor,
                                                I&& interpreter) noexcept {
  return detail::either_parser<E,
                               I,
                               decltype(interpreter(descriptor.left())),
                               decltype(interpreter(descriptor.right()))>{
      interpreter(descriptor.left()), interpreter(descriptor.right())

  };
}

template <class I>
constexpr auto parsers_interpreters_make_parser(
    [[maybe_unused]] description::succeed_t,
    [[maybe_unused]] I&& interpreter) noexcept {
  return detail::succeed_parser<I>{};
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
          detail::instance_of<S, description::static_string> = 0>
constexpr auto parsers_interpreters_make_parser(
    S&& descriptor,
    [[maybe_unused]] I interpreter) noexcept {
  return detail::static_string_parser<std::decay_t<S>, I>{
      std::forward<S>(descriptor)};
}

template <class S, class I, detail::instance_of<S, description::sequence> = 0>
constexpr auto parsers_interpreters_make_parser(
    S&& descriptor,
    [[maybe_unused]] I&& interpreter) noexcept {
  return detail::sequence_parser<std::decay_t<S>, std::decay_t<I>>{
      std::forward<S>(descriptor), std::forward<I>(interpreter)};
}

namespace detail {}  // namespace detail

template <class A,
          class I,
          detail::instance_of<A, description::alternative> = 0>
constexpr auto parsers_interpreters_make_parser(
    A&& descriptor,
    [[maybe_unused]] I&& interpreter) noexcept {
  return detail::alternative_parser<std::decay_t<A>, std::decay_t<I>>{
      std::forward<A>(descriptor), std::forward<I>(interpreter)};
}

}  // namespace parsers::customization_points

#endif  // PARSERS_CUSTOMIZATION_POINTS_HPP