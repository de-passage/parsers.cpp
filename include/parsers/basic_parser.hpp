#ifndef GUARD_PARSERS_BASIC_PARSER_HPP
#define GUARD_PARSERS_BASIC_PARSER_HPP

#include "./description.hpp"
#include "./result.hpp"
#include "./utility.hpp"

#include <optional>
#include <type_traits>

namespace parsers {
namespace customization_points {
namespace detail {
using namespace ::parsers::detail;

template <class I, class R, class T>
using result_t = typename std::decay_t<I>::template result_t<std::decay_t<R>,
                                                             std::decay_t<T>>;

template <class I, class T, class B, class E>
[[nodiscard]] constexpr auto success(B&& before, B&& after, E&& end) noexcept {
  return std::decay_t<I>::template success<std::decay_t<T>>(
      std::forward<B>(before), std::forward<B>(after), std::forward<E>(end));
}

template <class I, class T, class B, class E>
[[nodiscard]] constexpr auto failure(B&& before, B&& after, E&& end) noexcept {
  return std::decay_t<I>::template failure<std::decay_t<T>>(
      std::forward<B>(before), std::forward<B>(after), std::forward<E>(end));
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
  return [](auto beg, auto end) -> std::optional<decltype(beg)> {
    if (beg == end) {
      return beg;
    }
    return {};
  };
}

template <class M, class I, detail::instance_of<M, description::many> = 0>
constexpr auto parsers_interpreters_make_parser(M&& descriptor,
                                                I interpreter) noexcept {
  return [parser = interpreter(descriptor.parser())](
             auto beg, auto end) -> std::optional<decltype(beg)> {
    while (beg != end) {
      auto r = parser(beg, end);
      if (!r.has_value()) {
        break;
      }
      beg = *r;
    }
    return {beg};
  };
}

template <class B, class I, detail::instance_of<B, description::both> = 0>
constexpr auto parsers_interpreters_make_parser(B&& descriptor,
                                                I interpreter) noexcept {
  return [left = interpreter(descriptor.left()),
          right = interpreter(descriptor.right())](
             auto beg, auto end) -> std::optional<decltype(beg)> {
    if (auto r1 = left(beg, end); r1.has_value()) {
      if (auto r2 = right(*r1, end); r2.has_value()) {
        return r2;
      }
    }
    return {};
  };
}

template <class E, class I, detail::instance_of<E, description::either> = 0>
constexpr auto parsers_interpreters_make_parser(E&& descriptor,
                                                I interpreter) noexcept {
  return [left = interpreter(descriptor.left()),
          right = interpreter(descriptor.right())](
             auto beg, auto end) -> std::optional<decltype(beg)> {
    if (auto r = left(beg, end); r.has_value()) {
      return r;
    }
    return right(beg, end);
  };
}

constexpr auto parsers_interpreters_make_parser(
    [[maybe_unused]] description::succeed_t) noexcept {
  return
      [](auto beg, [[maybe_unused]] auto end) -> std::optional<decltype(beg)> {
        return beg;
      };
}

template <class R, class I>
constexpr auto parsers_interpreters_make_parser(
    description::recursive<R> descriptor,
    I interpreter) noexcept {
  return detail::parser_indirection_t<typename decltype(descriptor)::parser_t,
                                      I>{descriptor.parser(), interpreter};
}

template <class S,
          class I,
          detail::instance_of<S, description::static_string> = 0>
constexpr auto parsers_interpreters_make_parser(
    S&& descriptor,
    [[maybe_unused]] I interpreter) noexcept {
  return [descriptor = std::forward<S>(descriptor)](
             auto beg, auto end) -> std::optional<decltype(beg)> {
    auto itb = descriptor.begin;
    while (itb != descriptor.end) {
      if (beg == end || *beg != *itb) {
        return {};
      }
      ++beg;
      ++itb;
    }
    return {beg};
  };
}
}  // namespace customization_points

namespace interpreters {
using ::parsers::customization_points::parsers_interpreters_make_parser;

struct range_parser {
  template <class I, class T = I>
  using result_t = dpsg::result<std::pair<I, I>, I>;

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB> success(ItB before,
                                                ItB after,
                                                [[maybe_unused]] ItE end) {
    return dpsg::success(before, after);
  }

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB> failure([[maybe_unused]] ItB before,
                                                ItB after,
                                                [[maybe_unused]] ItE end) {
    return dpsg::failure(after);
  }
};

struct object_parser {
  template <class I, class T>
  using result_t = dpsg::result<std::pair<I, description::object_t<T, I>>, I>;

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB, T> success(ItB before,
                                                   ItB after,
                                                   [[maybe_unused]] ItE end) {
    return dpsg::success(after, T::build(before, after));
  }

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB, T> failure([[maybe_unused]] ItB before,
                                                   ItB after,
                                                   [[maybe_unused]] ItE end) {
    return dpsg::failure(after);
  }
};

namespace detail {
template <class Traits>
struct make_parser_t : Traits {
  template <class T>
  [[nodiscard]] constexpr auto operator()(T&& descriptor) const noexcept {
    return parsers_interpreters_make_parser(std::forward<T>(descriptor), *this);
  }
};
}  // namespace detail

template <class T>
using make_parser_t = detail::make_parser_t<T>;

template <class T>
constexpr static inline make_parser_t<T> make_parser{};
}  // namespace interpreters
}  // namespace parsers

#endif  // GUARD_PARSERS_BASIC_PARSER_HPP