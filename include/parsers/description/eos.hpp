#ifndef GUARD_PARSERS_DESCRIPTION_EOS_HPP
#define GUARD_PARSERS_DESCRIPTION_EOS_HPP

#include "./discard.hpp"
#include "./satisfy.hpp"

namespace parsers::description {
namespace detail {
struct is_zero : description::satisfy_character<is_zero> {
  template <class C>
  [[nodiscard]] constexpr bool operator()(C c) const noexcept {
    return c == static_cast<C>(0);
  }
};
}  // namespace detail

using eos_t = description::discard<
    description::either<description::end_t, detail::is_zero>>;
constexpr static inline eos_t eos{};
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_EOS_HPP