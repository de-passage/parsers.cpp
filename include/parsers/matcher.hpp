#ifndef GUARD_PARSERS_INTERPRETERS_MATCHER_HPP
#define GUARD_PARSERS_INTERPRETERS_MATCHER_HPP

#include "./description.hpp"
#include "parsers/is_template_instance.hpp"

#include <cctype>
#include <optional>
#include <type_traits>

namespace parsers {
namespace customization_points {
constexpr auto parsers_interpreters_make_matcher(
    [[maybe_unused]] description::fail) noexcept {
  return []([[maybe_unused]] auto beg,
            [[maybe_unused]] auto end) -> std::optional<decltype(beg)> {
    return {};
  };
}

template <class T,
          std::enable_if_t<description::is_satisfiable_predicate_v<T>, int> = 0>
constexpr auto parsers_interpreters_make_matcher(T pred) {
  return [pred](auto beg, auto end) -> std::optional<decltype(beg)> {
    if (beg != end && pred(*beg)) {
      return {++beg};
    }
    return {};
  };
}

constexpr auto parsers_interpreters_make_matcher(
    [[maybe_unused]] description::end_t) noexcept {
  return [](auto beg, auto end) -> std::optional<decltype(beg)> {
    if (beg == end) {
      return beg;
    }
    return {};
  };
}

namespace detail {
template <class T, template <class...> class I>
using instance_of =
    std::enable_if_t<dpsg::is_template_instance_v<std::decay_t<T>, I>, int>;
}

template <class M, class I, detail::instance_of<M, description::many> = 0>
constexpr auto parsers_interpreters_make_matcher(M&& descriptor,
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
constexpr auto parsers_interpreters_make_matcher(B&& descriptor,
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
constexpr auto parsers_interpreters_make_matcher(E&& descriptor,
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

constexpr auto parsers_interpreters_make_matcher(
    [[maybe_unused]] description::succeed_t) noexcept {
  return
      [](auto beg, [[maybe_unused]] auto end) -> std::optional<decltype(beg)> {
        return beg;
      };
}

namespace detail {
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
}  // namespace detail

template <class R, class I>
constexpr auto parsers_interpreters_make_matcher(
    description::recursive<R> descriptor,
    I interpreter) noexcept {
  return detail::parser_indirection_t<typename decltype(descriptor)::parser_t,
                                      I>{descriptor.parser(), interpreter};
}

template <class S,
          class I,
          detail::instance_of<S, description::static_string> = 0>
constexpr auto parsers_interpreters_make_matcher(S&& descriptor,
                                                 I interpreter) noexcept {
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

namespace detail {
using ::parsers::customization_points::parsers_interpreters_make_matcher;

template <class T, class = void>
struct overloard_takes_single_argument : std::false_type {};
template <class T>
struct overloard_takes_single_argument<
    T,
    std::void_t<decltype(parsers_interpreters_make_matcher(std::declval<T>()))>>
    : std::true_type {};
template <class T>
constexpr static inline bool overload_takes_single_argument_v =
    overloard_takes_single_argument<T>::value;

struct make_matcher_t {
 public:
  template <class T>
  [[nodiscard]] constexpr auto operator()(T descriptor) const noexcept {
    if constexpr (overload_takes_single_argument_v<T>) {
      return parsers_interpreters_make_matcher(descriptor);
    }
    else {
      return parsers_interpreters_make_matcher(descriptor, *this);
    }
  }
};
}  // namespace detail

constexpr static inline detail::make_matcher_t make_matcher{};
}  // namespace interpreters

}  // namespace parsers

#endif  // GUARD_PARSERS_INTERPRETERS_MATCHER_HPP