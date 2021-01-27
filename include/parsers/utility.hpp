#ifndef GUARD_PARSERS_UTILITY_HPP
#define GUARD_PARSERS_UTILITY_HPP

#include "./is_template_instance.hpp"

#include <utility>

namespace parsers {
struct empty {};

namespace detail {
template <class T, template <class...> class I>
using instance_of =
    std::enable_if_t<dpsg::is_template_instance_v<std::decay_t<T>, I>, int>;

template <class D, class I>
struct parser_indirection_t {
  using parser_t = D;
  using interpreter_t = I;
  constexpr explicit parser_indirection_t(parser_t parser,
                                          interpreter_t interpreter) noexcept
      : parser{parser}, interpreter{interpreter} {}

  template <class K, class J>
  [[nodiscard]] constexpr inline auto operator()(K begin,
                                                 J end) const noexcept {
    if constexpr (std::is_invocable_v<interpreter_t, parser_t, interpreter_t>) {
      return interpreter(parser, interpreter)(begin, end);
    }
    else {
      return interpreter(parser)(begin, end);
    }
  }

 private:
  parser_t parser;
  interpreter_t interpreter;
};
}  // namespace detail
}  // namespace parsers

#endif  // GUARD_PARSERS_UTILITY_HPP