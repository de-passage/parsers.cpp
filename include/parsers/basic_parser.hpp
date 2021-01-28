#ifndef GUARD_PARSERS_BASIC_PARSER_HPP
#define GUARD_PARSERS_BASIC_PARSER_HPP

#include "./description.hpp"
#include "./result.hpp"
#include "./utility.hpp"

#include <optional>
#include <type_traits>
#include <vector>

namespace parsers {
namespace customization_points {
namespace detail {
using namespace ::parsers::detail;

template <class I, class R, class T>
using result_t = typename std::decay_t<I>::template result_t<std::decay_t<R>,
                                                             std::decay_t<T>>;

template <class I, class T, class... Args>
[[nodiscard]] constexpr auto success(Args&&... args) noexcept {
  return std::decay_t<I>::template success(type<std::decay_t<T>>,
                                           std::forward<Args>(args)...);
}

template <class I, class T, class... Args>
[[nodiscard]] constexpr auto failure(Args&&... args) noexcept {
  return std::decay_t<I>::template failure(type<std::decay_t<T>>,
                                           std::forward<Args>(args)...);
}

template <class I, class T, class... Args>
[[nodiscard]] constexpr auto combine(Args&&... args) noexcept {
  return std::decay_t<I>::template combine(type<std::decay_t<T>>,
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
    auto acc = detail::success<I, description::many>(beg, beg, end);
    while (beg != end) {
      auto r = parser(beg, end);
      if (!r.has_value()) {
        break;
      }
      beg = r->first;
    }
    return {beg};
  };
}

template <class B, class I, detail::instance_of<B, description::both> = 0>
constexpr auto parsers_interpreters_make_parser(B&& descriptor,
                                                I interpreter) noexcept {
  return [left = interpreter(descriptor.left()),
          right = interpreter(descriptor.right())](
             auto beg, auto end) -> detail::result_t<I, decltype(beg), B> {
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
                                                I&& interpreter) noexcept {
  return [left = interpreter(descriptor.left()),
          right = interpreter(descriptor.right())](
             auto beg, auto end) -> detail::result_t<I, decltype(beg), E> {
    if (auto r = left(beg, end); r.has_value()) {
      return r;
    }
    return right(beg, end);
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
}  // namespace customization_points

namespace interpreters {
using ::parsers::customization_points::parsers_interpreters_make_parser;

struct range_parser {
  template <class I, class T = I>
  using result_t = dpsg::result<std::pair<I, I>, I>;

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB> success([[maybe_unused]] type_t<T>,
                                                ItB before,
                                                ItB after,
                                                [[maybe_unused]] ItE end) {
    return dpsg::success(before, after);
  }

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB> failure([[maybe_unused]] type_t<T>,
                                                [[maybe_unused]] ItB before,
                                                ItB after,
                                                [[maybe_unused]] ItE end) {
    return dpsg::failure(after);
  }
};

struct object_parser {
  template <class T, class I>
  struct object {
    using type = description::object_t<std::decay_t<T>, std::decay_t<I>>;
  };
  template <class T, class C, class I>
  struct object<description::many<T, C>, I> {
    using type =
        std::vector<typename object<std::decay_t<T>, std::decay_t<I>>::type>;
  };

  template <class T, class I>
  using object_t = typename object<std::decay_t<T>, std::decay_t<I>>::type;

  template <class I, class T>
  using result_t = dpsg::result<std::pair<I, object_t<T, I>>, I>;

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB, T> success([[maybe_unused]] type_t<T>,
                                                   ItB before,
                                                   ItB after,
                                                   [[maybe_unused]] ItE end) {
    return dpsg::success(after, T::build(before, after));
  }

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB, T> failure([[maybe_unused]] type_t<T>,
                                                   [[maybe_unused]] ItB before,
                                                   ItB after,
                                                   [[maybe_unused]] ItE end) {
    return dpsg::failure(after);
  }

  template <class T, class I, class... Args>
  constexpr static inline object_t<T, I> build(
      [[maybe_unused]] type_t<description::many<T>>,
      [[maybe_unused]] I&& it,
      [[maybe_unused]] Args&&...) noexcept {
    return object_t<T, I>{};
  }

  template <class T, class Acc, class Add>
  constexpr static inline void combine(
      [[maybe_unused]] type_t<description::many<T>>,
      Acc& acc,
      Add&& add) noexcept {
    acc.push_back(std::forward<Add>(add));
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