#include <cstddef>
#include <iterator>
#include <parsers/parsers.hpp>

#include <parsers/description/bind.hpp>

#include <gtest/gtest.h>

namespace example {
using namespace parsers::description;
using namespace parsers::dsl;

struct n_char {
  char c;
  constexpr auto operator()(int n) const noexcept {
    return at_least{static_cast<std::size_t>(n), c};
  }
};

struct to_int {
  constexpr int operator()(char c) const noexcept { return c - '0'; }
} constexpr to_int;

struct next_char {
  constexpr auto operator()(char c) const noexcept {
    return character_class{[c](char d) { return c + 1 == d; }};
  }
} constexpr next_char;

struct to_string {
  constexpr static_string<char> operator()(const char* s,
                                           const char* e) const noexcept {
    return static_string{s, e};
  }
} constexpr to_string;

struct reversed {
  struct iterator {
    using value_type = const char;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = const char*;
    using reference = const char&;

    const char* ptr;
    std::ptrdiff_t dist;
    constexpr iterator& operator++() {
      --dist;
      return *this;
    }
    constexpr char operator*() const { return *(ptr + dist - 1); }
    friend constexpr bool operator==(const iterator& l,
                                     const iterator& r) noexcept {
      return l.ptr == r.ptr && l.dist == r.dist;
    }
    friend constexpr bool operator!=(const iterator& l,
                                     const iterator& r) noexcept {
      return !(l == r);
    }
  };
  constexpr auto operator()(static_string<char> str) const noexcept {
    const auto b = str.begin();
    const auto e = str.end();
    const auto d = std::distance(b, e);
    return static_string(iterator{b, d}, iterator{b, 0});
  }
} constexpr reversed;

constexpr auto _1 = bind{map{ascii::digit, to_int}, n_char{'a'}};
constexpr auto _2 = bind{any, next_char};
constexpr auto _3 =
    bind{both{build{many{ascii::alpha}, to_string}, discard{ascii::space}},
         reversed};

}  // namespace example

TEST(Bind, ShouldWorkUnchangedWithMatcher) {
  using namespace example;
  static_assert(
      std::is_same_v<decltype(parsers::interpreters::object_parser::value(
                         _1.interpret()(std::declval<char*>(),
                                        std::declval<char*>()))),
                     int>);

  static_assert(parsers::match(_1, "3aaa"));
  static_assert(!parsers::match(_1, "3aa"));
  static_assert(!parsers::match(_2, ""));
  static_assert(parsers::match(_2, "ab"));
  static_assert(parsers::match(_2, "cd"));
  static_assert(!parsers::match(_2, "c"));
  static_assert(parsers::match(_3, "test tset"));
  static_assert(parsers::match(_3, "palindrome emordnilap"));
  static_assert(!parsers::match(_3, "fail fail"));
}