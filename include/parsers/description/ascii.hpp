#ifndef GUARD_PARSERS_DESCRIPTION_ASCII_HPP
#define GUARD_PARSERS_DESCRIPTION_ASCII_HPP

#include "./char_utils.hpp"
#include "./satisfy.hpp"

namespace parsers::description::ascii {

template <class T>
struct character_class : satisfy_character<character_class<T>>, T {
  using T::operator();
};

struct cntrl_t : character_class<characters::ascii::is_cntrl_t> {
} constexpr static inline cntrl;
struct print_t : character_class<characters::ascii::is_print_t> {
} constexpr static inline print;
struct space_t : character_class<characters::ascii::is_space_t> {
} constexpr static inline space;
struct blank_t : character_class<characters::ascii::is_blank_t> {
} constexpr static inline blank;
struct graph_t : character_class<characters::ascii::is_graph_t> {
} constexpr static inline graph;
struct punct_t : character_class<characters::ascii::is_punct_t> {
} constexpr static inline punct;
struct alnum_t : character_class<characters::ascii::is_alnum_t> {
} constexpr static inline alnum;
struct alpha_t : character_class<characters::ascii::is_alpha_t> {
} constexpr static inline alpha;
struct upper_t : character_class<characters::ascii::is_upper_t> {
} constexpr static inline upper;
struct lower_t : character_class<characters::ascii::is_lower_t> {
} constexpr static inline lower;
struct digit_t : character_class<characters::ascii::is_digit_t> {
} constexpr static inline digit;
struct xdigit_t : character_class<characters::ascii::is_xdigit_t> {
} constexpr static inline xdigit;

template <auto Char, class T = decltype(Char)>
struct case_insensitive_character
    : satisfy_character<case_insensitive_character<Char, T>> {
  using base = satisfy_character<case_insensitive_character<Char, T>>;

  constexpr case_insensitive_character() noexcept = default;

  template <class C>
  constexpr bool operator()(C c) const noexcept {
    return c == Char || characters::ascii::toggle_case(Char) == c;
  }
};
template <class C>
struct case_insensitive_character<nullptr, detail::dynamic<C>>
    : satisfy_character<
          case_insensitive_character<nullptr, detail::dynamic<C>>> {
  using base = satisfy_character<
      case_insensitive_character<nullptr, detail::dynamic<C>>>;

  constexpr case_insensitive_character() noexcept = default;

  template <class D,
            std::enable_if_t<
                !std::is_same_v<std::decay_t<D>, case_insensitive_character>,
                int> = 0>
  constexpr case_insensitive_character(D&& d) : _value(std::forward<D>(d)) {}

  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c == _value || characters::ascii::toggle_case(_value) == c;
  }

 private:
  C _value;
};
template <class C>
case_insensitive_character(C&& c)
    -> case_insensitive_character<nullptr, detail::dynamic<std::decay_t<C>>>;

template <class T>
using dynamic_case_insensitive_character =
    case_insensitive_character<nullptr, detail::dynamic<T>>;

}  // namespace parsers::description::ascii
#endif  // GUARD_PARSERS_DESCRIPTION_ASCII_HPP