#ifndef GUARD_PARSERS_BASIC_PARSER_HPP
#define GUARD_PARSERS_BASIC_PARSER_HPP

#include "../description.hpp"
#include "../utility.hpp"

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace parsers {

namespace interpreters {
namespace detail {
using namespace ::parsers::detail;

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
    using type = typename t<T>::type;
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
    return std::ref(add);
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

  template <class S, class... Args>
  constexpr static inline auto sequence([[maybe_unused]] type_t<S>,
                                        Args&&... args) noexcept {
    return dpsg::success(
        std::get<0>(detail::last_of(std::forward<Args>(args)...).value()),
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
};
}  // namespace interpreters
}  // namespace parsers

#endif  // GUARD_PARSERS_BASIC_PARSER_HPP