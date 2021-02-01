#ifndef GUARD_PARSERS_BASIC_PARSER_HPP
#define GUARD_PARSERS_BASIC_PARSER_HPP

#include "./description.hpp"
#include "./result.hpp"
#include "./utility.hpp"

#include <memory>
#include <optional>
#include <type_traits>
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
          std::enable_if_t<description::is_satisfiable_predicate_v<S>, int> = 0>
constexpr static inline auto build([[maybe_unused]] type_t<S>,
                                   I&& b,
                                   [[maybe_unused]] I&& e) noexcept {
  return *std::forward<I>(b);
}

template <auto V,
          class C,
          class I,
          std::enable_if_t<!std::is_same_v<decltype(V), nullptr_t>, int> = 0>
constexpr static inline typename description::character<V, C>::value_type build(
    [[maybe_unused]] type_t<description::character<V, C>>,
    [[maybe_unused]] I&& b,
    [[maybe_unused]] I&& e) noexcept {
  return description::character<V, C>::value;
}

template <class C, class I>
constexpr static inline typename description::character<nullptr, C>::value_type
build([[maybe_unused]] type_t<description::character<nullptr, C>>,
      [[maybe_unused]] I&& b,
      [[maybe_unused]] I&& e) noexcept {
  return *std::forward<I>(b);
}

template <class I>
constexpr static inline
    typename std::iterator_traits<std::decay_t<I>>::value_type
    build([[maybe_unused]] type_t<description::any_t>,
          I&& b,
          [[maybe_unused]] I&& e) noexcept {
  return *std::forward<I>(b);
}

template <class I>
constexpr static inline empty build(
    [[maybe_unused]] type_t<description::succeed_t>,
    [[maybe_unused]] I&& b,
    [[maybe_unused]] I&& e) noexcept {
  return {};
}

template <class S, class I, instance_of<S, description::static_string> = 0>
constexpr static inline S build([[maybe_unused]] type_t<S>,
                                [[maybe_unused]] I&& b,
                                [[maybe_unused]] I&& e) noexcept {
  return S{std::forward<I>(b), std::forward<I>(e)};
}

template <class I>
constexpr static inline empty build([[maybe_unused]] type_t<description::end_t>,
                                    [[maybe_unused]] I&& b,
                                    [[maybe_unused]] I&& e) noexcept {
  return {};
}

template <class F,
          class I,
          std::enable_if_t<description::is_failure_v<F>, int> = 0>
constexpr static inline empty build([[maybe_unused]] type_t<F>,
                                    [[maybe_unused]] I&&,
                                    [[maybe_unused]] I&&) noexcept;

template <class T, class I>
struct object {
  using type = decltype(build(type<T>, std::declval<I>(), std::declval<I>()));
};
template <class T, class I>
using object_t = typename object<T, I>::type;
}  // namespace detail

struct object_parser {
  template <class T, class I>
  struct object {
    using type = typename std::conditional_t<
        description::is_recursive_v<T>,
        type_t<detail::unique_ptr<T, object_parser, I>>,
        detail::object<std::decay_t<T>, std::decay_t<I>>>::type;
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
  template <class I, class... Args>
  struct object<description::sequence<Args...>, I> {
    template <class T>
    using t_ = typename object<std::decay_t<T>, std::decay_t<I>>::type;
    using type = std::tuple<t_<Args>...>;
  };

  template <class I, class T>
  using object_t = typename object<std::decay_t<T>, std::decay_t<I>>::type;

  template <class I, class T>
  using result_t = dpsg::result<std::pair<I, object_t<I, T>>, I>;

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB, T> success(
      [[maybe_unused]] type_t<T> tag,
      ItB before,
      ItB after,
      [[maybe_unused]] ItE end) {
    using ::parsers::interpreters::detail::build;
    return dpsg::success(after, build(tag, before, after));
  }

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB, T> failure([[maybe_unused]] type_t<T>,
                                                   [[maybe_unused]] ItB before,
                                                   ItB after,
                                                   [[maybe_unused]] ItE end) {
    return dpsg::failure(after);
  }

  template <class... Ms, class ItB, class ItE>
  constexpr static inline result_t<std::decay_t<ItB>, description::many<Ms...>>
  init([[maybe_unused]] type_t<description::many<Ms...>>,
       [[maybe_unused]] ItB&&,
       [[maybe_unused]] ItE&&) noexcept {
    return dpsg::success();
  }

  template <class T, class C, class Acc, class Add>
  constexpr static inline auto combine(
      [[maybe_unused]] type_t<description::many<T, C>>,
      Acc& acc,
      Add&& add) noexcept {
    if (add.has_value()) {
      acc.value().second.push_back(std::forward<Add>(add).value().second);
      acc.value().first = std::forward<Add>(add).value().first;
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

  template <class R, class E, detail::instance_of<E, description::either> = 0>
  constexpr static inline auto left(type_t<E>, R&& r) noexcept {
    return dpsg::success(
        std::forward<R>(r).value().first,
        object_t<std::decay_t<decltype(r.value().first)>, E>{
            std::in_place_index<0>, std::get<1>(std::forward<R>(r).value())});
  }

  template <class R, class E, detail::instance_of<E, description::either> = 0>
  constexpr static inline auto right(type_t<E>, R&& r) noexcept {
    return dpsg::success(
        std::get<0>(std::forward<R>(r).value()),
        object_t<std::decay_t<decltype(r.value().first)>, E>{
            std::in_place_index<1>, std::get<1>(std::forward<R>(r).value())});
  }

  template <class L,
            class R,
            class B,
            detail::instance_of<B, description::both> = 0>
  constexpr static inline auto both(type_t<B>, L&& left, R&& right) noexcept {
    return dpsg::success(std::get<0>(std::forward<R>(right).value()),
                         object_t<decltype(right.value().first), B>{
                             std::get<1>(std::forward<L>(left).value()),
                             std::get<1>(std::forward<R>(right).value())});
  }

  template <class S,
            class... Args,
            detail::instance_of<S, description::sequence> = 0>
  constexpr static inline auto sequence([[maybe_unused]] type_t<S>,
                                        Args&&... args) noexcept {
    return dpsg::success(
        std::get<0>(detail::last_of(std::forward<Args>(args)...).value()),
        object_t<decltype(detail::last_of(args...).value().first), S>{
            std::get<1>(std::forward<Args>(args).value())...});
  }
};
}  // namespace interpreters
}  // namespace parsers

#endif  // GUARD_PARSERS_BASIC_PARSER_HPP