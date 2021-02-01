#ifndef PARSERS_CUSTOMIZATION_POINTS_HPP
#define PARSERS_CUSTOMIZATION_POINTS_HPP

#include "./description.hpp"
#include "./result.hpp"
#include "./utility.hpp"

namespace parsers::customization_points {
namespace detail {
using namespace ::parsers::detail;

template <class D, class I>
struct parser_indirection_t {
  using parser_t = D;
  using interpreter_t = I;
  constexpr explicit parser_indirection_t(parser_t parser,
                                          interpreter_t interpreter) noexcept
      : parser{parser}, interpreter{interpreter} {}

  template <class K, class J>
  [[nodiscard]] constexpr inline auto operator()(K begin,
                                                 J end) const noexcept {
    if constexpr (std::is_invocable_v<interpreter_t, parser_t, interpreter_t>) {
      return interpreter(parser, interpreter)(begin, end);
    }
    else {
      return interpreter(parser)(begin, end);
    }
  }

 private:
  parser_t parser;
  interpreter_t interpreter;
};

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
}  // namespace detail

template <class F,
          class I,
          std::enable_if_t<description::is_failure_v<F>, int> = 0>
constexpr auto parsers_interpreters_make_parser([[maybe_unused]] F&&,
                                                [[maybe_unused]] I&&) noexcept {
  return [](auto beg, auto end) -> detail::result_t<I, decltype(beg), F> {
    return detail::failure<I, F>(beg, beg, end);
  };
}

template <class T,
          class I,
          std::enable_if_t<description::is_satisfiable_predicate_v<T>, int> = 0>
constexpr auto parsers_interpreters_make_parser(T&& pred,
                                                [[maybe_unused]] I&&) {
  return [pred](auto beg, auto end) -> detail::result_t<I, decltype(beg), T> {
    if (beg != end && pred(*beg)) {
      auto before = beg;
      return detail::success<I, T>(before, ++beg, end);
    }
    return detail::failure<I, T>(beg, beg, end);
  };
}

template <class I>
constexpr auto parsers_interpreters_make_parser(
    [[maybe_unused]] description::end_t,
    [[maybe_unused]] I&&) noexcept {
  return
      [](auto beg,
         auto end) -> detail::result_t<I, decltype(beg), description::end_t> {
        if (beg == end) {
          return detail::success<I, description::end_t>(beg, beg, end);
        }
        return detail::failure<I, description::end_t>(beg, beg, end);
      };
}

template <class M, class I, detail::instance_of<M, description::many> = 0>
constexpr auto parsers_interpreters_make_parser(M&& descriptor,
                                                I interpreter) noexcept {
  return [parser = interpreter(descriptor.parser())](
             auto beg, auto end) -> detail::result_t<I, decltype(beg), M> {
    auto acc = detail::init<I, M>(beg, beg);
    while (beg != end) {
      auto r = detail::combine<I, M>(acc, parser(beg, end));
      if (!detail::has_value<I>(r)) {
        break;
      }
      beg = detail::next_iterator<I>(std::move(r));
    }
    return acc;
  };
}

template <class B, class I, detail::instance_of<B, description::both> = 0>
constexpr auto parsers_interpreters_make_parser(B&& descriptor,
                                                I interpreter) noexcept {
  return [left = interpreter(descriptor.left()),
          right = interpreter(descriptor.right())](
             auto beg, auto end) -> detail::result_t<I, decltype(beg), B> {
    if (auto r1 = left(beg, end); detail::has_value<I>(r1)) {
      if (auto r2 = right(detail::next_iterator<I>(r1), end);
          detail::has_value<I>(r2)) {
        return detail::both<I, B>(std::move(r1), std::move(r2));
      }
      return detail::failure<I, B>(beg, detail::next_iterator<I>(r1), end);
    }
    return detail::failure<I, B>(beg, beg, end);
  };
}

template <class E, class I, detail::instance_of<E, description::either> = 0>
constexpr auto parsers_interpreters_make_parser(E&& descriptor,
                                                I&& interpreter) noexcept {
  return [left = interpreter(descriptor.left()),
          right = interpreter(descriptor.right())](
             auto beg, auto end) -> detail::result_t<I, decltype(beg), E> {
    if (auto l = left(beg, end); detail::has_value<I>(l)) {
      return detail::left<I, E>(std::move(l));
    }
    auto r = right(beg, end);
    if (detail::has_value<I>(r)) {
      return detail::right<I, E>(std::move(r));
    }
    return detail::failure<I, E>(beg, beg, end);
  };
}

template <class I>
constexpr auto parsers_interpreters_make_parser(
    [[maybe_unused]] description::succeed_t,
    [[maybe_unused]] I&& interpreter) noexcept {
  return [](auto beg, auto end)
             -> detail::result_t<I, decltype(beg), description::succeed_t> {
    return detail::success<I, description::succeed_t>(beg, beg, end);
  };
}

template <class R, class I>
constexpr auto parsers_interpreters_make_parser(
    description::recursive<R> descriptor,
    I&& interpreter) noexcept {
  return detail::parser_indirection_t<typename decltype(descriptor)::parser_t,
                                      std::decay_t<I>>{
      descriptor.parser(), std::forward<I>(interpreter)};
}

template <class S,
          class I,
          detail::instance_of<S, description::static_string> = 0>
constexpr auto parsers_interpreters_make_parser(
    S&& descriptor,
    [[maybe_unused]] I interpreter) noexcept {
  return [descriptor = std::forward<S>(descriptor)](
             auto beg, auto e) -> detail::result_t<I, decltype(beg), S> {
    using std::end;
    using std::begin;
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
  };
}

namespace detail {

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

}  // namespace detail

template <class S, class I, detail::instance_of<S, description::sequence> = 0>
constexpr auto parsers_interpreters_make_parser(
    S&& descriptor,
    [[maybe_unused]] I&& interpreter) noexcept {
  return [descriptor = std::forward<S>(descriptor),
          interpreter = std::forward<I>(interpreter)](
             auto beg, auto e) -> detail::result_t<I, decltype(beg), S> {
    return detail::call_sequence<0>(
        std::move(descriptor), std::move(interpreter), beg, beg, e);
  };
}

}  // namespace parsers::customization_points

#endif  // PARSERS_CUSTOMIZATION_POINTS_HPP