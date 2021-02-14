#ifndef GUARD_PARSERS_DESCRIPTION_STATIC_STRING_HPP
#define GUARD_PARSERS_DESCRIPTION_STATIC_STRING_HPP

#include "../utility.hpp"

#include "./char_utils.hpp"
#include "./satisfy.hpp"

#include <iterator>

namespace parsers::description {

namespace detail {
template <class Char, class It = const Char*>
struct basic_static_string {
  using char_t = Char;
  using pointer_t = It;
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

template <class Char, class It = const Char*>
struct static_string : satisfy<static_string<Char, It>>,
                       detail::basic_static_string<Char, It> {
  using base = detail::basic_static_string<Char, It>;
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
template <class T>
static_string(T&&, T&&)
    -> static_string<typename std::iterator_traits<std::decay_t<T>>::value_type,
                     std::decay_t<T>>;

namespace ascii {
namespace detail {

template <class Char, class It, class CRTP>
struct case_insensitive_static_string
    : satisfy<case_insensitive_static_string<Char, It, CRTP>>,
      ::parsers::description::detail::basic_static_string<Char, It> {
  using base = ::parsers::description::detail::basic_static_string<Char, It>;
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

template <class Char, class It = const Char*>
struct case_insensitive_static_string
    : detail::case_insensitive_static_string<
          Char,
          It,
          case_insensitive_static_string<Char>> {};

template <class It>
struct case_insensitive_static_string<char, It>
    : detail::case_insensitive_static_string<
          char,
          It,
          case_insensitive_static_string<char>> {
  using base = detail::case_insensitive_static_string<
      char,
      It,
      case_insensitive_static_string<char>>;
  template <class... Args>
  constexpr explicit case_insensitive_static_string(Args&&... args) noexcept
      : base{std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr char convert(char c) const noexcept {
    return ::characters::ascii::toggle_case(c);
  }
};
template <class T, std::size_t S>
case_insensitive_static_string(T (&)[S])
    -> case_insensitive_static_string<std::decay_t<T>>;
template <class T>
case_insensitive_static_string(const T*, const T*)
    -> case_insensitive_static_string<T>;
template <class T>
case_insensitive_static_string(T&&, T&&) -> case_insensitive_static_string<
    typename std::iterator_traits<std::decay_t<T>>::value_type,
    std::decay_t<T>>;
}  // namespace ascii

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_STATIC_STRING_HPP