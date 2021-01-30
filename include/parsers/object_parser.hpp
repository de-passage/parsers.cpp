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
struct unique_ptr {
  using type = unique_ptr<T, P, I>;

  template <class U>
  unique_ptr(U&& u) : _ptr{new U(std::forward<U>(u))} {
    static_assert(
        std::is_same_v<std::decay_t<U>, recursive_pointer_type<T, P, I>>);
  }

  operator bool() const noexcept { return !!_ptr; }

  template <class U>
  friend bool operator==(const unique_ptr& ptr, U&& other) noexcept {
    return _ptr == std::forward<U>(other);
  }

  template <class U>
  friend bool operator==(U&& other, const unique_ptr& ptr) noexcept {
    return _ptr == std::forward<U>(other);
  }

  friend bool operator==(const unique_ptr& left,
                         const unique_ptr& right) noexcept {
    return left._ptr == right._ptr;
  }

  template <class U>
  friend bool operator!=(const unique_ptr& ptr, U&& other) noexcept {
    return _ptr != std::forward<U>(other);
  }

  template <class U>
  friend bool operator!=(U&& other, const unique_ptr& ptr) noexcept {
    return _ptr != std::forward<U>(other);
  }

  friend bool operator!=(const unique_ptr& left,
                         const unique_ptr& right) noexcept {
    return left._ptr != right._ptr;
  }

  [[nodiscard]] auto* get() const {
    return static_cast<recursive_pointer_type<T, P, I>*>(_ptr.get());
  }

  auto* operator->() const noexcept { return get(); }

  [[nodiscard]] auto& operator*() const noexcept { return *get(); }

  [[nodiscard]] auto* release() noexcept {
    return static_cast<recursive_pointer_type<T, P, I>*>(_ptr.release());
  }

 private:
  struct deleter {
    void operator()(void* ptr) {
      delete static_cast<recursive_pointer_type<T, P, I>*>(ptr);
    }
  };
  using pointer = std::unique_ptr<void, deleter>;
  pointer _ptr;
};
}  // namespace detail

struct object_parser {
  template <class T, class I>
  struct object {
    using type = typename std::conditional_t<
        description::is_recursive_v<T>,
        detail::unique_ptr<T, object_parser, I>,
        description::object<std::decay_t<T>, std::decay_t<I>>>::type;
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
            std::in_place_index<0>, std::forward<R>(r).value().second});
  }

  template <class R, class E, detail::instance_of<E, description::either> = 0>
  constexpr static inline auto right(type_t<E>, R&& r) noexcept {
    return dpsg::success(
        std::forward<R>(r).value().first,
        object_t<std::decay_t<decltype(r.value().first)>, E>{
            std::in_place_index<1>, std::forward<R>(r).value().second});
  }

  template <class L,
            class R,
            class B,
            detail::instance_of<B, description::both> = 0>
  constexpr static inline auto both(type_t<B>, L&& left, R&& right) noexcept {
    return dpsg::success(
        std::forward<R>(right).value().first,
        object_t<std::decay_t<decltype(right.value().first)>, B>{
            std::forward<L>(left).value().second,
            std::forward<R>(right).value().second});
  }
};
}  // namespace interpreters
}  // namespace parsers

#endif  // GUARD_PARSERS_BASIC_PARSER_HPP