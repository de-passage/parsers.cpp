#ifndef GUARD_PARSERS_DESCRIPTION_STATIC_STRING_HPP
#define GUARD_PARSERS_DESCRIPTION_STATIC_STRING_HPP

#include "../utility.hpp"

#include "./satisfy.hpp"

namespace parsers::description {

namespace detail {
template <class Char>
struct basic_static_string {
  using char_t = Char;
  using pointer_t = const Char*;
  template <class T, std::size_t S>
  constexpr explicit basic_static_string(T (&arr)[S]) noexcept
      : _begin(static_cast<pointer_t>(arr)),
        _end(static_cast<pointer_t>(arr) + S - 1) {}

  constexpr basic_static_string(pointer_t beg, pointer_t end) noexcept
      : _begin(beg), _end(end) {}

  constexpr auto begin() const noexcept { return _begin; }
  constexpr auto cbegin() const noexcept { return _begin; }
  constexpr auto end() const noexcept { return _end; }
  constexpr auto cend() const noexcept { return _end; }

 private:
  pointer_t _begin;
  pointer_t _end;
};

}  // namespace detail

template <class Char>
struct static_string : satisfy<static_string<Char>>,
                       detail::basic_static_string<Char> {
  using base = detail::basic_static_string<Char>;
  using pointer_t = typename base::pointer_t;

  constexpr static_string(pointer_t beg, pointer_t end) noexcept
      : base{beg, end} {}

  template <class T, std::size_t S>
  constexpr explicit static_string(T (&arr)[S]) noexcept : base{arr} {}

  template <class U, class V>
  [[nodiscard]] constexpr auto operator()(U begin, V end) const noexcept {
    auto beg = begin;
    auto itb = base::begin();
    while (itb != base::end()) {
      if (beg == end || *beg != *itb) {
        return begin;
      }
      ++beg;
      ++itb;
    }
    return beg;
  }
};
template <class T, std::size_t S>
static_string(T (&)[S]) -> static_string<std::decay_t<T>>;
template <class T>
static_string(const T*, const T*) -> static_string<T>;

namespace detail {

template <class Char, class CRTP>
struct case_insensitive_static_string
    : satisfy<case_insensitive_static_string<Char, CRTP>>,
      basic_static_string<Char> {
  using base = basic_static_string<Char>;
  using pointer_t = typename base::pointer_t;

  constexpr case_insensitive_static_string(pointer_t beg,
                                           pointer_t end) noexcept
      : base{beg, end} {}

  template <class T, std::size_t S>
  constexpr explicit case_insensitive_static_string(T (&arr)[S]) noexcept
      : base{arr} {}

  template <class U, class V>
  [[nodiscard]] constexpr auto operator()(U begin, V end) const noexcept {
    auto beg = begin;
    auto itb = base::begin();
    while (itb != base::end()) {
      if (beg == end ||
          (*beg != *itb &&
           static_cast<const CRTP*>(this)->convert(*beg) != *itb)) {
        return begin;
      }
      ++beg;
      ++itb;
    }
    return beg;
  }
};

}  // namespace detail

template <class Char>
struct case_insensitive_static_string
    : detail::case_insensitive_static_string<
          Char,
          case_insensitive_static_string<Char>> {};

namespace detail {
struct conversion_table_t {
  constexpr conversion_table_t() noexcept {
    for (unsigned char c = 0; c < 128; ++c) {
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        storage[c] = static_cast<char>(c | 32);
      }
      else {
        storage[c] = static_cast<char>(c);
      }
    }
  }
  char storage[128]{};
};
constexpr static inline detail::conversion_table_t conversion_table{};
}  // namespace detail

template <>
struct case_insensitive_static_string<char>
    : detail::case_insensitive_static_string<
          char,
          case_insensitive_static_string<char>> {
  using base = detail::case_insensitive_static_string<
      char,
      case_insensitive_static_string<char>>;
  template <class... Args>
  constexpr explicit case_insensitive_static_string(Args&&... args) noexcept
      : base{std::forward<Args>(args)...} {}

  constexpr char convert(char c) const noexcept {
    return detail::conversion_table.storage[static_cast<unsigned char>(c)];
  }
};
template <class T, std::size_t S>
case_insensitive_static_string(T (&)[S])
    -> case_insensitive_static_string<std::decay_t<T>>;
template <class T>
case_insensitive_static_string(const T*, const T*)
    -> case_insensitive_static_string<T>;
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_STATIC_STRING_HPP