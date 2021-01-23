#ifndef GUARD_PARSERS_INTERPRETERS_MATCHER_HPP
#define GUARD_PARSERS_INTERPRETERS_MATCHER_HPP

#include "./description.hpp"

#include <optional>
#include <type_traits>

namespace parsers {
namespace customization_points {
constexpr auto parsers_interpreters_make_matcher(
    [[maybe_unused]] description::fail) noexcept {
  return []([[maybe_unused]] auto beg,
            [[maybe_unused]] auto end) -> std::optional<decltype(beg)> {
    return {};
  };
}

template <class T,
          std::enable_if_t<description::is_satisfiable_predicate_v<T>, int> = 0>
constexpr auto parsers_interpreters_make_matcher(T pred) {
  return [pred](auto beg, auto end) -> std::optional<decltype(beg)> {
    if (beg != end && pred(*beg)) {
      return {++beg};
    }
    return {};
  };
}

constexpr auto parsers_interpreters_make_matcher(
    [[maybe_unused]] description::end) noexcept {
  return []([[maybe_unused]] auto beg,
            [[maybe_unused]] auto end) -> std::optional<decltype(beg)> {
    if (beg == end) {
      return beg;
    }
    return {};
  };
}

template <class T>
constexpr auto parsers_interpreters_make_matcher(
    description::many<T> descriptor) noexcept {
  return [parser = make_parser(descriptor.parser())](
             [[maybe_unused]] auto beg,
             [[maybe_unused]] auto end) -> std::optional<decltype(beg)> {
    while (beg != end) {
      auto r = parser(beg, end);
      if (!r.has_value()) {
        break;
      }
      beg = *r;
    }
    return {beg};
  };
}
}  // namespace customization_points

namespace interpreters {

namespace detail {
using ::parsers::customization_points::parsers_interpreters_make_matcher;
struct make_matcher_t {
  template <class T>
  [[nodiscard]] constexpr auto operator()(T descriptor) const noexcept {
    return parsers_interpreters_make_matcher(descriptor);
  }
};
}  // namespace detail

constexpr static inline detail::make_matcher_t make_matcher{};
}  // namespace interpreters

}  // namespace parsers

#endif  // GUARD_PARSERS_INTERPRETERS_MATCHER_HPP