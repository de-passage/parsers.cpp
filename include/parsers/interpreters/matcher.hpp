#ifndef GUARD_PARSERS_INTERPRETERS_MATCHER_HPP
#define GUARD_PARSERS_INTERPRETERS_MATCHER_HPP

#include "../description.hpp"
#include "../utility.hpp"

#include <optional>
#include <type_traits>

namespace parsers::interpreters {
namespace detail {
using namespace ::parsers::detail;
}  // namespace detail

struct matcher {
  template <class I, class T = I>
  using result_t = std::optional<std::decay_t<I>>;

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB> success(
      [[maybe_unused]] type_t<T>,
      [[maybe_unused]] ItB before,
      ItB after,
      [[maybe_unused]] ItE end) noexcept {
    return {after};
  }

  template <class T, class ItB, class ItE>
  constexpr static inline result_t<ItB> failure(
      [[maybe_unused]] type_t<T>,
      [[maybe_unused]] ItB before,
      [[maybe_unused]] ItB after,
      [[maybe_unused]] ItE end) noexcept {
    return {};
  }

  template <class M, class Acc, class Add>
  constexpr static inline auto combine([[maybe_unused]] type_t<M>,
                                       Acc& acc,
                                       Add&& add) noexcept {
    if (add.has_value()) {
      acc = *std::forward<Add>(add);
    }
    return add;
  }

  template <class R>
  constexpr static inline auto next_iterator(R&& r) noexcept {
    return *std::forward<R>(r);
  }

  template <class R>
  constexpr static inline bool has_value(R&& r) noexcept {
    return std::forward<R>(r).has_value();
  }

  template <class S, class... Args>
  constexpr static inline auto sequence([[maybe_unused]] type_t<S> s,
                                        Args&&... args) noexcept {
    return detail::last_of(std::forward<Args>(args)...);
  }

  template <std::size_t S, class D, class I>
  constexpr static inline auto alternative([[maybe_unused]] type_t<D>,
                                           I&& input) noexcept {
    return input;
  }

  template <class M, class IB, class IE, class E, class D = std::decay_t<M>>
  constexpr static inline auto modify([[maybe_unused]] type_t<D>,
                                      [[maybe_unused]] M&&,
                                      dpsg::result<std::pair<IB, IB>, E>&& r,
                                      [[maybe_unused]] IB beg,
                                      [[maybe_unused]] IE end) noexcept {
    if (r.has_value()) {
      return result_t<IB>{std::get<1>(std::move(r).value())};
    }
    return result_t<IB>{};
  }

  template <class M,
            class IB,
            class IE,
            class E,
            class T,
            class D = std::decay_t<M>,
            std::enable_if_t<!std::is_same_v<std::decay_t<IB>, std::decay_t<T>>,
                             int> = 0>
  constexpr static inline auto modify([[maybe_unused]] type_t<D>,
                                      [[maybe_unused]] M&&,
                                      dpsg::result<std::pair<IB, T>, E>&& r,
                                      [[maybe_unused]] IB beg,
                                      [[maybe_unused]] IE end) noexcept {
    if (r.has_value()) {
      return result_t<IB>{std::get<0>(std::move(r).value())};
    }
    return result_t<IB>{};
  }
};

}  // namespace parsers::interpreters

#endif  // GUARD_PARSERS_INTERPRETERS_MATCHER_HPP