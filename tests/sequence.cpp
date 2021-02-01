#include <parsers/parsers.hpp>

#include <gtest/gtest.h>
#include <string>
#include "./streq.hpp"

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

TEST(Sequence, CanBeParsedForObjects) {
  using parsers::parse;
  using parsers::description::character;
  using parsers::description::either;
  using parsers::description::sequence;

  constexpr auto s =
      sequence{"Hello"_s, either{character{'!'}, " World!"_s}, eos};

  constexpr auto p1 = parse(s, "Hello World!");
  static_assert(p1.has_value());
  constexpr auto r1 = p1.value().second;
  static_assert(std::tuple_size<decltype(r1)>::value == 3);
  static_assert(streq(std::get<0>(r1), "Hello"));
  static_assert(std::get<1>(r1).index() == 1);
  static_assert(streq(std::get<1>(std::get<1>(r1)), "World!"));

  constexpr auto p2 = parse(s, "Hello!");
  static_assert(p2.has_value());
  constexpr auto r2 = p2.value().second;
  static_assert(std::tuple_size<decltype(r2)>::value == 3);
  static_assert(streq(std::get<0>(r2), "Hello"));
  static_assert(std::get<1>(r2).index() == 0);
  static_assert(std::get<0>(std::get<1>(r2)) == '!');

  constexpr auto p3 = parse(s, "HelloWorld!");
  static_assert(p3.is_error());
}