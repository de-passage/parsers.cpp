#ifndef GUARD_PARSERS_DESCRIPTION_ASCII_HPP
#define GUARD_PARSERS_DESCRIPTION_ASCII_HPP

#include "./char_utils.hpp"
#include "./satisfy.hpp"


namespace parsers::description::ascii {

struct digit : satisfy_character<digit> {
  template <class T>
  constexpr bool operator()(T d) const noexcept {
    return d >= '0' && d <= '9';
  }
} constexpr static inline isdigit;

struct whitespace : satisfy_character<whitespace> {
  template <class T>
  constexpr bool operator()(T w) const noexcept {
    return w == 32 || (w >= 9 && w <= 13);
  }
} constexpr static inline isspace;

struct control_character : satisfy_character<control_character> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 0 && c <= 31) || c == 127;
  }
} constexpr static inline isctrl;

struct printable_character : satisfy_character<printable_character> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c >= 32 && c <= 126;
  }
} constexpr static inline isprint;

struct blank : satisfy_character<blank> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c == 9 || c == 32;
  }
} constexpr static inline isblank;

struct graph : satisfy_character<graph> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c >= 33 && c <= 126;
  }
} constexpr static inline isgraph;

struct punctuation : satisfy_character<punctuation> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 33 && c <= 47) || (c >= 58 && c <= 64) ||
           (c >= 91 && c <= 96) || (c >= 123 && c <= 126);
  }
} constexpr static inline ispunc;

struct alphanum : satisfy_character<alphanum> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) ||
           (c >= 97 && c <= 122);
  }
} constexpr static inline isalphanum;

struct alpha : satisfy_character<alpha> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
  }
} constexpr static inline isalpha;

struct uppercase : satisfy_character<uppercase> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 65 && c <= 90);
  }
} constexpr static inline isupper;

struct lowercase : satisfy_character<lowercase> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 97 && c <= 122);
  }
} constexpr static inline islower;

struct hexdigit : satisfy_character<hexdigit> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 48 && c <= 57) || (c >= 65 && c <= 70) ||
           (c >= 97 && c <= 102);
  }
} constexpr static inline ishex;

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