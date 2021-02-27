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
  struct eq_next {
    char c;
    constexpr bool operator()(char d) const { return c + 1 == d; }
  };
  constexpr auto operator()(char c) const noexcept {
    return character_class{eq_next{c}};
  }
} constexpr next_char;

struct to_string {
  constexpr auto operator()(const char* s, const char* e) const noexcept {
    return parsers::range{s, e};
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
  template <class S>
  constexpr auto operator()(S str) const noexcept {
    const auto b = str.begin();
    const auto e = str.end();
    const auto d = std::distance(b, e);
    return parsers::description::static_string(iterator{b, d}, iterator{b, 0});
  }
} constexpr reversed;

constexpr auto _1 = ascii::digit / to_int >>= n_char{'a'};
constexpr auto _2 = any >>= next_char;
constexpr auto _3 = (many{ascii::alpha} /= to_string) & (~ascii::space) >>=
    reversed;

static_assert(
    std::is_same_v<decltype(_1)::template final_parser_type<char*, char*>,
                   at_least<parsers::description::detail::dynamic_count,
                            char,
                            container<char>>>);
static_assert(
    std::is_same_v<decltype(_2)::template final_parser_type<char*, char*>,
                   character_class<next_char::eq_next>>);

}  // namespace example

TEST(Bind, ShouldWorkUnchangedWithMatcher) {
  using namespace example;

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

template <class P, class T>
constexpr bool check(P&& pair, T&& str) noexcept {
  return parsers::range{pair.first, pair.second} == std::forward<T>(str);
}

TEST(Bind, ShouldWorkWithRange) {
  using namespace example;
  constexpr auto p1 = parsers::parse_range(_1, "3aaabc");
  static_assert(p1.has_value());
  constexpr auto r1 = p1.value();
  static_assert(check(r1, "aaa"));
  constexpr auto p2 = parsers::parse_range(_1, "3aa");
  static_assert(!p2.has_value());
  constexpr auto p3 = parsers::parse_range(_2, "");
  static_assert(!p3.has_value());
  constexpr auto p4 = parsers::parse_range(_2, "ab");
  static_assert(p4.has_value());
  static_assert(check(p4.value(), "b"));
  constexpr auto p5 = parsers::parse_range(_2, "ce");
  static_assert(!p5.has_value());
  constexpr auto p6 = parsers::parse_range(_2, "c");
  static_assert(!p6.has_value());
  constexpr auto p7 = parsers::parse_range(_3, "test tset");
  static_assert(p7.has_value());
  static_assert(check(p7.value(), "tset"));
  constexpr auto p8 = parsers::parse_range(_3, "palindrome emordnilap");
  static_assert(p8.has_value());
  static_assert(check(p8.value(), "emordnilap"));
  constexpr auto p9 = parsers::parse_range(_3, "fail fail");
  static_assert(!p9.has_value());
}

TEST(Bind, ObjectParserShouldWork) {
  using namespace example;

  auto p1 = parsers::parse(_1, "4aaaaaefd");
  ASSERT_TRUE(p1.has_value());
  auto& r1 = p1.value();
  ASSERT_EQ(r1.size(), 5);
  for (std::size_t s = 0; s < 5; ++s) {
    ASSERT_EQ(r1[s], 'a');
  }

  auto p2 = parsers::parse(many{_2}, "abcdef");
  ASSERT_TRUE(p2.has_value());
  auto& r2 = p2.value();
  ASSERT_EQ(r2.size(), 3);
  auto expected = "bdf";
  for (std::size_t s = 0; s < 3; ++s) {
    ASSERT_EQ(r2[s], expected[s]);
  }
}