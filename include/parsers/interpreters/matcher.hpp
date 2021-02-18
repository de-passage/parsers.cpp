#ifndef GUARD_PARSERS_INTERPRETERS_MATCHER_HPP
#define GUARD_PARSERS_INTERPRETERS_MATCHER_HPP

#include "../description.hpp"
#include "../utility.hpp"

#include <optional>
#include <type_traits>

#include "../result_traits.hpp"

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

  template <class I,
            class D,
            class IB,
            class IE,
            class D1 = detail::remove_cvref_t<D>>
  constexpr static inline result_t<IB> modify([[maybe_unused]] type_t<D1>,
                                              I&& interpreter,
                                              D&& description,
                                              IB beg,
                                              IE end) noexcept {
    auto result = interpreter(description.inner_parser())(beg, end);
    using traits = parsers::result_traits<decltype(result)>;
    if (traits::has_value(result)) {
      return result_t<IB>{traits::next_iterator(std::move(result))};
    }
    return result_t<IB>{};
  }
};

}  // namespace parsers::interpreters

#endif  // GUARD_PARSERS_INTERPRETERS_MATCHER_HPP