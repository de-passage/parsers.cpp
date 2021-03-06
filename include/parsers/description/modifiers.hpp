#ifndef GUARD_PARSERS_DESCRIPTION_MODIFIERS_HPP
#define GUARD_PARSERS_DESCRIPTION_MODIFIERS_HPP

#include "../utility.hpp"
#include "./containers.hpp"

#include <type_traits>

namespace parsers::description {

template <class T, class I>
struct modifier : private container<T> {
  using base = container<T>;
  using interpreter_t = I;
  using inner_parser_t = typename base::parser_t;

  constexpr modifier() noexcept = default;

  template <class U,
            std::enable_if_t<std::is_convertible_v<T, U> &&
                                 !std::is_same_v<std::decay_t<U>, modifier>,
                             int> = 0>
  constexpr explicit modifier(U&& modifier) noexcept
      : base{std::forward<U>(modifier)} {}

  friend constexpr std::true_type is_modifier_f(const modifier&) noexcept;

  constexpr interpreter_t& interpreter() & noexcept { return _interpreter; }

  constexpr const interpreter_t& interpreter() const& noexcept {
    return _interpreter;
  }

  constexpr interpreter_t&& interpreter() && noexcept {
    return static_cast<modifier&&>(*this)._interpreter;
  }

  constexpr const interpreter_t&& interpreter() const&& noexcept {
    return static_cast<const modifier&&>(*this)._interpreter;
  }

  constexpr inner_parser_t inner_parser() const { return base::parser(); }

 private:
  interpreter_t _interpreter;
};

constexpr std::false_type is_modifier_f(...) noexcept;
template <class T>
using is_modifier = decltype(is_modifier_f(std::declval<T>()));
template <class T>
constexpr static inline bool is_modifier_v = is_modifier<T>::value;

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_MODIFIERS_HPP