#ifndef GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP
#define GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP

#include "./description.hpp"
#include "./result.hpp"
#include "./utility.hpp"

#include <type_traits>

namespace parsers::interpreters {
namespace detail {
using namespace ::parsers::detail;
}  // namespace detail

struct range_parser {
  template <class I, class T = I>
  using result_t = dpsg::result<std::pair<std::decay_t<I>, std::decay_t<I>>,
                                std::decay_t<I>>;

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

  template <class... Ms, class ItE, class ItB>
  constexpr static inline result_t<ItB> init(
      [[maybe_unused]] type_t<description::many<Ms...>>,
      ItB&& beg,
      ItE&& end) noexcept {
    return dpsg::success(std::forward<ItB>(beg), std::forward<ItE>(end));
  }

  template <class T, class C, class Acc, class Add>
  constexpr static inline auto combine(
      [[maybe_unused]] type_t<description::many<T, C>>,
      Acc& acc,
      Add&& add) noexcept {
    if (add.has_value()) {
      acc.value().second = std::get<1>(std::forward<Add>(add).value());
    }
    return add;
  }

  template <class R, detail::not_instance_of<R, detail::reference_wrapper> = 0>
  constexpr static inline auto next_iterator(R&& r) noexcept {
    return std::forward<R>(r).value().second;
  }

  template <class R, detail::not_instance_of<R, detail::reference_wrapper> = 0>
  constexpr static inline bool has_value(R&& r) noexcept {
    return std::forward<R>(r).has_value();
  }

  template <class R, detail::instance_of<R, detail::reference_wrapper> = 0>
  constexpr static inline auto next_iterator(R&& r) noexcept {
    return std::forward<R>(r)->value().second;
  }

  template <class R, detail::instance_of<R, detail::reference_wrapper> = 0>
  constexpr static inline bool has_value(R&& r) noexcept {
    return std::forward<R>(r)->has_value();
  }

  template <class R, class E, detail::instance_of<E, description::either> = 0>
  constexpr static inline auto left(type_t<E>, R&& r) noexcept {
    return std::forward<R>(r);
  }

  template <class R, class E, detail::instance_of<E, description::either> = 0>
  constexpr static inline auto right(type_t<E>, R&& r) noexcept {
    return std::forward<R>(r);
  }

  template <class L,
            class R,
            class B,
            detail::instance_of<B, description::both> = 0>
  constexpr static inline auto both(type_t<B>, L&& left, R&& right) noexcept {
    return dpsg::success(std::get<0>(std::forward<L>(left).value()),
                         std::get<1>(std::forward<R>(right).value()));
  }

  template <class S, class A, class... Args>
  constexpr static inline auto sequence([[maybe_unused]] type_t<S> s,
                                        A&& a,
                                        Args&&... args) noexcept {
    return dpsg::success(
        std::get<0>(std::forward<A>(a).value()),
        std::get<1>(detail::last_of(std::forward<Args>(args)...).value()));
  }
};

}  // namespace parsers::interpreters

#endif  // GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP