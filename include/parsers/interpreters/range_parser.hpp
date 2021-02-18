#ifndef GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP
#define GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP

#include "../description.hpp"
#include "../utility.hpp"

#include <optional>
#include <type_traits>

#include "../result_traits.hpp"

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

  template <class M, class Acc, class Add>
  constexpr static inline auto combine([[maybe_unused]] type_t<M>,
                                       Acc& acc,
                                       Add&& add) noexcept {
    if (add.has_value()) {
      acc.value().second = std::get<1>(std::forward<Add>(add).value());
    }
    return std::forward<Add>(add);
  }

  template <class S, class A, class... Args>
  constexpr static inline auto sequence([[maybe_unused]] type_t<S> s,
                                        A&& a,
                                        Args&&... args) noexcept {
    return dpsg::success(
        std::get<0>(std::forward<A>(a).value()),
        std::get<1>(detail::last_of(std::forward<Args>(args)...).value()));
  }

  template <std::size_t S, class D, class I>
  constexpr static inline auto alternative([[maybe_unused]] type_t<D>,
                                           I&& value) noexcept {
    return std::forward<I>(value);
  }

  template <class I,
            class D,
            class ItB,
            class ItE,
            class D1 = detail::remove_cvref_t<D>>
  constexpr static inline auto modify([[maybe_unused]] type_t<D1>,
                                      I&& interpreter,
                                      D&& description,
                                      ItB begin,
                                      ItE end) noexcept -> result_t<ItB> {
    auto result = interpreter(description.parser())(begin, end);
    using traits = parsers::result_traits<decltype(result)>;
    if (!traits::has_value(result)) {
      return dpsg::failure(begin);
    }
    return dpsg::success(begin, traits::next_iterator(std::move(result)));
  }
};

}  // namespace parsers::interpreters

#endif  // GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP