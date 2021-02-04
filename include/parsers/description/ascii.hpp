#ifndef GUARD_PARSERS_DESCRIPTION_ASCII_HPP
#define GUARD_PARSERS_DESCRIPTION_ASCII_HPP

#include "./satisfy.hpp"

namespace parsers::description::ascii {

struct digit : satisfy<digit> {
  template <class T>
  constexpr bool operator()(T d) const noexcept {
    return d >= '0' && d <= '9';
  }
} constexpr static inline isdigit;

struct whitespace : satisfy<whitespace> {
  template <class T>
  constexpr bool operator()(T w) const noexcept {
    return w == 32 || (w >= 9 && w <= 13);
  }
} constexpr static inline isspace;

struct control_character : satisfy<control_character> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 0 && c <= 31) || c == 127;
  }
} constexpr static inline isctrl;

struct printable_character : satisfy<printable_character> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c >= 32 && c <= 126;
  }
} constexpr static inline isprint;

struct blank : satisfy<blank> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c == 9 || c == 32;
  }
} constexpr static inline isblank;

struct graph : satisfy<graph> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c >= 33 && c <= 126;
  }
} constexpr static inline isgraph;

struct punctuation : satisfy<punctuation> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 33 && c <= 47) || (c >= 58 && c <= 64) ||
           (c >= 91 && c <= 96) || (c >= 123 && c <= 126);
  }
} constexpr static inline ispunc;

struct alphanum : satisfy<alphanum> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) ||
           (c >= 97 && c <= 122);
  }
} constexpr static inline isalphanum;

struct alpha : satisfy<alpha> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
  }
} constexpr static inline isalpha;

struct uppercase : satisfy<uppercase> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 65 && c <= 90);
  }
} constexpr static inline isupper;

struct lowercase : satisfy<lowercase> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 97 && c <= 122);
  }
} constexpr static inline islower;

struct hexdigit : satisfy<hexdigit> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return (c >= 48 && c <= 57) || (c >= 65 && c <= 70) ||
           (c >= 97 && c <= 102);
  }
} constexpr static inline ishex;
}  // namespace parsers::description::ascii
#endif  // GUARD_PARSERS_DESCRIPTION_ASCII_HPP