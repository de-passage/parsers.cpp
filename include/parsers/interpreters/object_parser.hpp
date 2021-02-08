#ifndef GUARD_PARSERS_BASIC_PARSER_HPP
#define GUARD_PARSERS_BASIC_PARSER_HPP

#include "../description.hpp"
#include "../utility.hpp"

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace parsers::interpreters {
namespace detail {
using namespace ::parsers::detail;

template <class... Args>
struct collapse_tuple_arguments;
template <class A, class... Args, class... Args2>
struct collapse_tuple_arguments<std::tuple<A, Args...>, std::tuple<Args2...>> {
  using type = typename collapse_tuple_arguments<std::tuple<Args...>,
                                                 std::tuple<Args2..., A>>::type;
};
template <class... Args, class... Args2>
struct collapse_tuple_arguments<std::tuple<parsers::empty, Args...>,
                                std::tuple<Args2...>> {
  using type = typename collapse_tuple_arguments<std::tuple<Args...>,
                                                 std::tuple<Args2...>>::type;
};
template <>
struct collapse_tuple_arguments<std::tuple<>, std::tuple<>> {
  using type = parsers::empty;
};
template <class A>
struct collapse_tuple_arguments<std::tuple<>, std::tuple<A>> {
  using type = A;
};
template <class A, class B, class... Args>
struct collapse_tuple_arguments<std::tuple<>, std::tuple<A, B, Args...>> {
  using type = std::tuple<A, B, Args...>;
};

template <class T, class P, class I>
using recursive_pointer_type =
    typename P::template object_t<I, typename T::parser_t>;
template <class T, class P, class I>
struct unique_ptr : std::unique_ptr<recursive_pointer_type<T, P, I>> {
  template <
      class U,
      std::enable_if_t<!std::is_same_v<std::decay_t<U>, unique_ptr>, int> = 0>
  unique_ptr(U&& u)
      : std::unique_ptr<recursive_pointer_type<T, P, I>>{
            std::make_unique<recursive_pointer_type<T, P, I>>(
                std::forward<U>(u))} {
    static_assert(
        std::is_same_v<std::decay_t<U>, recursive_pointer_type<T, P, I>>);
  }

  unique_ptr(unique_ptr&& ptr) noexcept = default;
  unique_ptr(const unique_ptr& ptr) noexcept = delete;
};

template <class I,
          class S,
          std::enable_if_t<description::is_satisfiable_predicate_v<S>, int> = 0,
          not_instance_of<S, description::static_string> = 0>
constexpr static inline auto build([[maybe_unused]] type_t<S>,
                                   I&& b,
                                   [[maybe_unused]] I&& e) noexcept {
  return *std::forward<I>(b);
}

template <class I, class S, std::enable_if_t<std::is_integral_v<S>, int> = 0>
constexpr static inline auto build([[maybe_unused]] type_t<S>,
                                   I&& b,
                                   [[maybe_unused]] I&& e) noexcept {
  return *std::forward<I>(b);
}

template <class S, class I, instance_of<S, description::static_string> = 0>
constexpr static inline S build([[maybe_unused]] type_t<S>,
                                [[maybe_unused]] I&& b,
                                [[maybe_unused]] I&& e) noexcept {
  return S{std::forward<I>(b), std::forward<I>(e)};
}

template <class T,
          class I,
          std::enable_if_t<description::is_guard_v<T>, int> = 0>
constexpr static inline empty build([[maybe_unused]] type_t<T>,
                                    [[maybe_unused]] I&& b,
                                    [[maybe_unused]] I&& e) noexcept {
  return {};
}

template <class T,
          class I,
          std::enable_if_t<description::is_modifier_v<T>, int> = 0>
constexpr static inline empty build([[maybe_unused]] type_t<T>,
                                    [[maybe_unused]] I&& b,
                                    [[maybe_unused]] I&& e) noexcept {
  return {};
}

template <class T, class I>
struct object {
  using type = decltype(build(type<T>, std::declval<I>(), std::declval<I>()));
};

template <class T, class I>
using object_t = typename object<T, I>::type;

template <class T, class B, class A, class = void>
struct buildable : std::false_type {};
template <class T, class B, class A>
struct buildable<
    T,
    B,
    A,
    std::void_t<decltype(
        build(std::declval<T>(), std::declval<B>(), std::declval<A>()))>>
    : std::true_type {};
}  // namespace detail

struct object_parser {
  template <class T, class I, class = void>
  struct object {
    using type = typename std::conditional_t<
        description::is_recursive_v<T>,
        type_t<detail::unique_ptr<T, object_parser, I>>,
        detail::object<std::decay_t<T>, std::decay_t<I>>>::type;
  };

  template <class I, class T>
  using object_t = typename object<std::decay_t<T>, std::decay_t<I>>::type;

  template <class C, class I>
  struct object<const C*, I, std::enable_if_t<std::is_integral_v<C>>> {
    using type = description::static_string<C>;
  };
  template <class M, class I>
  struct object<M, I, std::enable_if_t<description::is_dynamic_range_v<M>>> {
    using type = std::vector<object_t<I, typename M::parser_t>>;
  };
  template <class I, class T>
  struct object<T, I, std::enable_if_t<description::is_sequence_v<T>>> {
    template <class U>
    struct t;
    template <template <class...> class C, class... Ts>
    struct t<C<Ts...>> {
      using type = std::tuple<object_t<I, Ts>...>;
    };
    using type = typename detail::collapse_tuple_arguments<typename t<T>::type,
                                                           std::tuple<>>::type;
  };
  template <class T, class I>
  struct object<T, I, std::enable_if_t<description::is_alternative_v<T>>> {
    template <class U>
    struct v;
    template <template <class...> class C, class... Ts>
    struct v<C<Ts...>> {
      using type = std::variant<object_t<I, Ts>...>;
    };
    using type = typename v<T>::type;
  };

  template <class M, class I>
  struct object<M, I, std::enable_if_t<description::is_modifier_v<M>>> {
    using type = empty;
  };

  template <class I, class T>
  using result_t = dpsg::result<std::pair<I, object_t<I, T>>, I>;

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB, T> success(
      [[maybe_unused]] type_t<T> tag,
      ItB before,
      ItB after,
      [[maybe_unused]] ItE end) {
    using ::parsers::interpreters::detail::build;
    if constexpr (detail::buildable<type_t<T>, ItB, ItB>::value) {
      return dpsg::success(after, build(tag, before, after));
    }
    else {
      return dpsg::success(
          std::piecewise_construct, std::tuple{after}, std::tuple{});
    }
  }

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB, T> failure([[maybe_unused]] type_t<T>,
                                                   [[maybe_unused]] ItB before,
                                                   ItB after,
                                                   [[maybe_unused]] ItE end) {
    return dpsg::failure(after);
  }

  template <class M, class Acc, class Add>
  constexpr static inline auto combine([[maybe_unused]] type_t<M>,
                                       Acc& acc,
                                       Add&& add) noexcept {
    if (add.has_value()) {
      acc.value().second.push_back(std::get<1>(std::forward<Add>(add).value()));
      acc.value().first = std::get<0>(std::forward<Add>(add).value());
    }
    return add;
  }

  template <class R>
  constexpr static inline auto next_iterator(R&& r) noexcept {
    return std::forward<R>(r).value().first;
  }

  template <class R>
  constexpr static inline bool has_value(R&& r) noexcept {
    return std::forward<R>(r).has_value();
  }

  template <class S>
  constexpr static inline auto sequence([[maybe_unused]] type_t<S>) noexcept {
    return dpsg::success(parsers::empty{});
  }

  template <class S, class A>
  constexpr static inline auto sequence([[maybe_unused]] type_t<S>,
                                        A&& arg) noexcept {
    return arg;
  }

  template <class S, class A, class... Args>
  constexpr static inline auto sequence([[maybe_unused]] type_t<S>,
                                        A&& arg,
                                        Args&&... args) noexcept {
    return dpsg::success(
        std::get<0>(arg.value()),
        object_t<decltype(detail::last_of(args...).value().first), S>{
            std::get<1>(std::forward<Args>(args).value())...});
  }

  template <std::size_t S, class D, class T>
  constexpr static inline auto alternative([[maybe_unused]] type_t<D>,
                                           T&& t) noexcept {
    return dpsg::success(
        std::get<0>(std::forward<T>(t).value()),
        object_t<decltype(t.value().first), D>{
            std::in_place_index<S>, std::get<1>(std::forward<T>(t).value())});
  }

  template <class D, class IB, class IE, class T>
  constexpr static inline result_t<IB, D> modify(
      [[maybe_unused]] type_t<D>,
      std::optional<T>&& r,
      IB begin,
      [[maybe_unused]] IE end) noexcept {
    if (r.has_value()) {
      return dpsg::success(std::pair<T, empty>{*r, {}});
    }
    else {
      return dpsg::failure(begin);
    }
  }
};
}  // namespace parsers::interpreters

#endif  // GUARD_PARSERS_BASIC_PARSER_HPP