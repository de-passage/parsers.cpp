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

  template <class... Ms, class ItE, class ItB>
  constexpr static inline result_t<ItB, description::many<Ms...>> init(
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
    return dpsg::success();
  }

  template <class R, class E, detail::instance_of<E, description::either> = 0>
  constexpr static inline auto right(type_t<E>, R&& r) noexcept {
    return dpsg::success();
  }

  template <class L,
            class R,
            class B,
            detail::instance_of<B, description::both> = 0>
  constexpr static inline auto both(type_t<B>, L&& left, R&& right) noexcept {
    return dpsg::success();
  }
};

}  // namespace parsers::interpreters

#endif  // GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP