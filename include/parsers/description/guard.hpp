#ifndef GUARD_PARSERS_DESCRIPTION_GUARD_HPP
#define GUARD_PARSERS_DESCRIPTION_GUARD_HPP

#include "../utility.hpp"

#include <type_traits>

namespace parsers::description {

template <class T>
struct fail_t {
  friend constexpr std::true_type is_failure_f(fail_t) noexcept;
};
constexpr std::false_type is_failure_f(...) noexcept;
template <class T>
using is_failure = decltype(is_failure_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_failure_v = is_failure<T>::value;

struct succeed_t {};

struct end_t {};
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_GUARD_HPP