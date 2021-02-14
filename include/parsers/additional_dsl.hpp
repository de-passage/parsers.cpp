#ifndef GUARD_PARSERS_ADDITIONAL_DSL_HPP
#define GUARD_PARSERS_ADDITIONAL_DSL_HPP

#include "./description/dependent_modifiers.hpp"

namespace parsers::dsl {

template <class T, class F>
constexpr auto operator>>=(T&& parser, F&& to_bind) noexcept {
  return parsers::description::bind{std::forward<T>(parser),
                                    std::forward<F>(to_bind)};
}

template <class T, class F>
constexpr auto operator/(T&& parser, F&& map) noexcept {
  return parsers::description::map{std::forward<T>(parser),
                                   std::forward<F>(map)};
}

template <class T, class F>
constexpr auto operator/=(T&& parser, F&& map) noexcept {
  return parsers::description::build{std::forward<T>(parser),
                                     std::forward<F>(map)};
}

template <class T>
constexpr auto operator~(T&& parser) noexcept {
  return parsers::description::discard{std::forward<T>(parser)};
}

}  // namespace parsers::dsl

#endif  // GUARD_PARSERS_ADDITIONAL_DSL_HPP