#ifndef GUARD_PARSERS_DESCRIPTION_GUARD_HPP
#define GUARD_PARSERS_DESCRIPTION_GUARD_HPP

#include "../utility.hpp"

#include <type_traits>

namespace parsers::description {

template <class T>
struct guard {
  friend constexpr std::true_type is_guard_f(const guard&) noexcept;

  template <class U, class V>
  [[nodiscard]] constexpr bool operator()(U&& begin, V&& end) const noexcept {
    return static_cast<T*>(this)->operator()(std::forward<U>(begin),
                                             std::forward<V>(end));
  }
};
constexpr std::false_type is_guard_f(...) noexcept;
template <class T>
using is_guard = decltype(is_guard_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_guard_v = is_guard<T>::value;

template <class T>
struct fail_t : guard<fail_t<T>> {
  friend constexpr std::true_type is_failure_f(fail_t) noexcept;

  template <class U, class V>
  [[nodiscard]] constexpr bool operator()(
      [[maybe_unused]] U&& begin,
      [[maybe_unused]] V&& end) const noexcept {
    return false;
  }
};
constexpr std::false_type is_failure_f(...) noexcept;
template <class T>
using is_failure = decltype(is_failure_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_failure_v = is_failure<T>::value;

struct succeed_t : guard<succeed_t> {
  template <class U, class V>
  [[nodiscard]] constexpr bool operator()(
      [[maybe_unused]] U&& u,
      [[maybe_unused]] V&& v) const noexcept {
    return true;
  }
};

struct end_t : guard<end_t> {
  template <class U, class V>
  [[nodiscard]] constexpr bool operator()(
      [[maybe_unused]] U&& begin,
      [[maybe_unused]] V&& end) const noexcept {
    return std::forward<U>(begin) == std::forward<V>(end);
  }
};
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_GUARD_HPP