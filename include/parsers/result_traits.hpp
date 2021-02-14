#ifndef GUARD_PARSERS_CUSTOMIZATION_POINTS_RESULT_TRAITS_HPP
#define GUARD_PARSERS_CUSTOMIZATION_POINTS_RESULT_TRAITS_HPP

#include "./utility.hpp"

#include <optional>

namespace parsers::customization_points {
template <class T>
struct result_traits {
 private:
  using type = T;

 public:
  using value_type = typename type::value_type;
  using iterator_type = typename type::iterator_type;
  using failure_type = typename type::failure_type;
};

template <class It>
struct result_traits<std::optional<It>> {
  using value_type = empty;
  using iterator_type = It;
  using failure_type = empty;

  template <class U,
            std::enable_if_t<std::is_same_v<std::optional<It>, std::decay_t<U>>,
                             int> = 0>
  constexpr static inline bool has_value(U&& result) noexcept {
    return result.has_value();
  }

  template <class U,
            std::enable_if_t<std::is_same_v<std::optional<It>, std::decay_t<U>>,
                             int> = 0>
  constexpr static inline value_type value(
      [[maybe_unused]] U&& result) noexcept {
    return empty{};
  }

  template <class U,
            std::enable_if_t<std::is_same_v<std::optional<It>, std::decay_t<U>>,
                             int> = 0>
  constexpr static inline iterator_type next_iterator(U&& result) noexcept {
    return *std::forward<U>(result);
  }

  template <class U,
            std::enable_if_t<std::is_same_v<std::optional<It>, std::decay_t<U>>,
                             int> = 0>
  constexpr static inline failure_type failure(
      [[maybe_unused]] U&& result) noexcept {
    return empty{};
  }
};

template <class It, class F>
struct result_traits<dpsg::result<std::pair<It, It>, F>> {
  using type = dpsg::result<std::pair<It, It>, F>;
  using value_type = std::pair<It, It>;
  using iterator_type = It;
  using failure_type = F;

  template <class U,
            std::enable_if_t<std::is_same_v<type, std::decay_t<U>>, int> = 0>
  constexpr static inline bool has_value(U&& result) noexcept {
    return result.has_value();
  }

  template <class U,
            std::enable_if_t<std::is_same_v<type, std::decay_t<U>>, int> = 0>
  constexpr static inline value_type value(U&& result) noexcept {
    return std::forward<U>(result).value();
  }

  template <class U,
            std::enable_if_t<std::is_same_v<type, std::decay_t<U>>, int> = 0>
  constexpr static inline iterator_type next_iterator(U&& result) noexcept {
    return std::get<1>(std::forward<U>(result).value());
  }

  template <class U,
            std::enable_if_t<std::is_same_v<type, std::decay_t<U>>, int> = 0>
  constexpr static inline failure_type failure(U&& result) noexcept {
    return std::forward<U>(result).error();
  }
};

template <class It, class T, class F>
struct result_traits<dpsg::result<std::pair<It, T>, F>> {
  using type = dpsg::result<std::pair<It, T>, F>;
  using value_type = T;
  using iterator_type = It;
  using failure_type = F;

  template <class U,
            std::enable_if_t<std::is_same_v<type, std::decay_t<U>>, int> = 0>
  constexpr static inline bool has_value(U&& result) noexcept {
    return result.has_value();
  }

  template <class U,
            std::enable_if_t<std::is_same_v<type, std::decay_t<U>>, int> = 0>
  constexpr static inline value_type value(U&& result) noexcept {
    return std::get<1>(std::forward<U>(result).value());
  }

  template <class U,
            std::enable_if_t<std::is_same_v<type, std::decay_t<U>>, int> = 0>
  constexpr static inline iterator_type next_iterator(U&& result) noexcept {
    return std::get<0>(std::forward<U>(result).value());
  }

  template <class U,
            std::enable_if_t<std::is_same_v<type, std::decay_t<U>>, int> = 0>
  constexpr static inline failure_type failure(U&& result) noexcept {
    return std::forward<U>(result).error();
  }
};
}  // namespace parsers::customization_points
namespace parsers {
template <class T>
using result_traits = customization_points::result_traits<T>;

template <class R>
[[nodiscard]] constexpr bool has_value(R&& result) noexcept {
  return result_traits<std::decay_t<R>>::has_value(std::forward<R>(result));
}

template <class R>
[[nodiscard]] constexpr typename result_traits<std::decay_t<R>>::value_type
value(R&& result) noexcept {
  return result_traits<std::decay_t<R>>::value(std::forward<R>(result));
}

template <class R>
[[nodiscard]] constexpr typename result_traits<std::decay_t<R>>::iterator_type
next_iterator(R&& result) noexcept {
  return result_traits<std::decay_t<R>>::next_iterator(std::forward<R>(result));
}
}  // namespace parsers

#endif  // GUARD_PARSERS_CUSTOMIZATION_POINTS_RESULT_TRAITS_HPP