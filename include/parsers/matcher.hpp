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

template <class T, class I>
constexpr auto parsers_interpreters_make_matcher(
    description::many<T> descriptor,
    I interpreter) noexcept {
  return [parser = interpreter(descriptor.parser())](
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

template <class T, class = void>
struct overloard_takes_single_argument : std::false_type {};
template <class T>
struct overloard_takes_single_argument<
    T,
    std::void_t<decltype(parsers_interpreters_make_matcher(std::declval<T>()))>>
    : std::true_type {};
template <class T>
constexpr static inline bool overload_takes_single_argument_v =
    overloard_takes_single_argument<T>::value;

struct make_matcher_t {
 public:
  template <class T>
  [[nodiscard]] constexpr auto operator()(T descriptor) const noexcept {
    if constexpr (overload_takes_single_argument_v<T>) {
      return parsers_interpreters_make_matcher(descriptor);
    }
    else {
      return parsers_interpreters_make_matcher(descriptor, *this);
    }
  }
};
}  // namespace detail

constexpr static inline detail::make_matcher_t make_matcher{};
}  // namespace interpreters

}  // namespace parsers

#endif  // GUARD_PARSERS_INTERPRETERS_MATCHER_HPP