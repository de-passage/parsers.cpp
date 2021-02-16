#ifndef GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP
#define GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP

#include "../description.hpp"
#include "../utility.hpp"

#include <optional>
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

  template <class M, class Acc, class Add>
  constexpr static inline auto combine([[maybe_unused]] type_t<M>,
                                       Acc& acc,
                                       Add&& add) noexcept {
    if (add.has_value()) {
      acc.value().second = std::get<1>(std::forward<Add>(add).value());
    }
    return std::move(add);
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
    return value;
  }

  template <class M, class I, class J, class D = std::decay_t<M>>
  constexpr static inline auto modify([[maybe_unused]] type_t<D>,
                                      [[maybe_unused]] M&&,
                                      std::optional<I>&& opt,
                                      I begin,
                                      [[maybe_unused]] J end) noexcept
      -> result_t<I> {
    if (!opt.has_value()) {
      return dpsg::failure(begin);
    }
    else {
      return dpsg::success(begin, *opt);
    }
  }

  template <class M,
            class I,
            class J,
            class T,
            class D = std::decay_t<M>,
            std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::decay_t<I>>,
                             int> = 0>
  constexpr static inline auto modify([[maybe_unused]] type_t<D>,
                                      [[maybe_unused]] M&&,
                                      dpsg::result<std::pair<I, T>, I>&& r,
                                      I begin,
                                      [[maybe_unused]] J end) noexcept
      -> result_t<I> {
    return std::move(r).map([begin = std::move(begin)](auto&& p) {
      return std::pair{std::move(begin),
                       std::get<0>(std::forward<decltype(p)>(p))};
    });
  }
};

}  // namespace parsers::interpreters

#endif  // GUARD_PARSERS_INTERPRETERS_RANGE_PARSER_HPP