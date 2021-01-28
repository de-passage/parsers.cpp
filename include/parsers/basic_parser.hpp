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

template <class I, class T, class U>
[[nodiscard]] constexpr auto init() noexcept {
  return std::decay_t<I>::init(type<std::decay_t<T>>, type<std::decay_t<U>>);
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
    auto acc = detail::init<I, M, decltype(beg)>();
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
      if (auto r2 = right(*r1, end); detail::has_value<I>(r2)) {
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
    if (auto l = left(beg, end); detail::has_value<I>(l)) {
      return detail::left<I, E>(l);
    }
    else {
      auto r = right(beg, end);
      if (detail::has_value<I>(r)) {
        return detail::right<I, E>(r);
      }
      return detail::failure<I, E>(beg, detail::next_iterator<I>(r), end);
    }
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
  template <class A, class B, class C, class I>
  struct object<description::either<A, B, C>, I> {
    template <class T>
    using t_ = typename object<std::decay_t<T>, std::decay_t<I>>::type;
    using type = std::variant<t_<A>, t_<B>>;
  };
  template <class A, class B, class C, class I>
  struct object<description::both<A, B, C>, I> {
    template <class T>
    using t_ = typename object<std::decay_t<T>, std::decay_t<I>>::type;
    using type = std::pair<t_<A>, t_<B>>;
  };

  template <class I, class T>
  using object_t = typename object<std::decay_t<T>, std::decay_t<I>>::type;

  template <class I, class T>
  using result_t = dpsg::result<std::pair<I, object_t<I, T>>, I>;

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

  template <class... Ms, class I>
  constexpr static inline result_t<I, description::many<Ms...>> init(
      [[maybe_unused]] type_t<description::many<Ms...>>,
      [[maybe_unused]] type_t<I>) noexcept {
    return dpsg::success();
  }

  template <class T, class C, class Acc, class Add>
  constexpr static inline auto combine(
      [[maybe_unused]] type_t<description::many<T, C>>,
      Acc& acc,
      Add&& add) noexcept {
    acc.value().second.push_back(std::forward<Add>(add).value().second);
    acc.value().first = std::forward<Add>(add).value().first;
    return std::ref(acc);
  }

  template <class R, detail::not_instance_of<R, std::reference_wrapper> = 0>
  constexpr static inline auto next_iterator(R&& r) noexcept {
    return std::forward<R>(r).value().first;
  }

  template <class R, detail::not_instance_of<R, std::reference_wrapper> = 0>
  constexpr static inline bool has_value(R&& r) noexcept {
    return std::forward<R>(r).has_value();
  }

  template <class R, detail::instance_of<R, std::reference_wrapper> = 0>
  constexpr static inline auto next_iterator(R&& r) noexcept {
    return std::forward<R>(r).get().value().first;
  }

  template <class R, detail::instance_of<R, std::reference_wrapper> = 0>
  constexpr static inline bool has_value(R&& r) noexcept {
    return std::forward<R>(r).get().has_value();
  }

  template <class R, class E, detail::instance_of<E, description::either> = 0>
  constexpr static inline auto left(type_t<E>, R&& r) noexcept {
    return dpsg::success(
        std::forward<R>(r).value().first,
        object_t<std::decay_t<decltype(r.value().first)>, E>{
            std::in_place_index<0>, std::forward<R>(r).value().second});
  }

  template <class R, class E, detail::instance_of<E, description::either> = 0>
  constexpr static inline auto right(type_t<E>, R&& r) noexcept {
    return dpsg::success(
        std::forward<R>(r).value().first,
        object_t<std::decay_t<decltype(r.value().first)>, E>{
            std::in_place_index<1>, std::forward<R>(r).value().second});
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