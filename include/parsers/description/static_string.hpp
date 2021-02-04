#ifndef GUARD_PARSERS_DESCRIPTION_STATIC_STRING_HPP
#define GUARD_PARSERS_DESCRIPTION_STATIC_STRING_HPP

#include "../utility.hpp"

namespace parsers::description {
template <class Char>
struct static_string {
  using char_t = Char;
  using pointer_t = const Char*;
  constexpr static_string(pointer_t beg, pointer_t end) noexcept
      : _begin(beg), _end(end) {}

  template <class T, std::size_t S>
  constexpr explicit static_string(T (&arr)[S]) noexcept
      : _begin(static_cast<pointer_t>(arr)),
        _end(static_cast<pointer_t>(arr) + S - 1) {}

  constexpr auto begin() const noexcept { return _begin; }
  constexpr auto cbegin() const noexcept { return _begin; }
  constexpr auto end() const noexcept { return _end; }
  constexpr auto cend() const noexcept { return _end; }

 private:
  pointer_t _begin;
  pointer_t _end;
};
template <class T, std::size_t S>
static_string(T (&)[S]) -> static_string<std::decay_t<T>>;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_STATIC_STRING_HPP