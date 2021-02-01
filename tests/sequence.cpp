#include <gtest/gtest.h>

#include <parsers/parsers.hpp>

#include <string>

using namespace std::literals::string_literals;
using namespace parsers::dsl;

TEST(Sequence, MayBeCtedEmpty) {
  using parsers::match;
  using parsers::description::character;
  using parsers::description::sequence;

  using s = sequence<character<'a'>, character<'b'>, character<'c'>>;

  static_assert(match(s{}, "abc"));
  static_assert(!match(s{}, "acb"));
  static_assert(!match(s{}, ""));

  auto str = "abcd"s;
  ASSERT_TRUE(match(s{}, str));
  ASSERT_EQ(parsers::match_length(s{}, str), 3);
};

TEST(Sequence, MayBeCtedFromParsers) {
  using parsers::match;
  using parsers::description::character;
  using parsers::description::sequence;

  constexpr auto s = sequence{character{'a'}, character{'b'}, character{'c'}};

  static_assert(match(s, "abc"));
  static_assert(!match(s, "acb"));
  static_assert(!match(s, ""));

  auto str = "abcd"s;
  ASSERT_TRUE(match(s, str));
  ASSERT_EQ(parsers::match_length(s, str), 3);
}