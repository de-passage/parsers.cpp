#include <parsers/parsers.hpp>

#include <gtest/gtest.h>
#include <string>

#include "./streq.hpp"

using namespace parsers::dsl;
using namespace std::literals::string_literals;
using namespace parsers::description;

using sstring = parsers::description::static_string<char>;
constexpr sstring string(
    const std::pair<const char*, const char*>& p) noexcept {
  return sstring{p.first, p.second};
}

template <class D, class S>
constexpr auto parse(D&& desc, S&& str) noexcept {
  using std::begin;
  using std::end;
  const auto p =
      parsers::interpreters::make_parser<parsers::interpreters::range_parser>(
          desc);
  return p(begin(str), end(str));
}

TEST(RangeParser, FailShouldMatchNothing) {
  constexpr auto p = parse(fail, "anything");
  static_assert(p.is_error());
}

TEST(RangeParser, AnyShouldMatchEverything) {
  constexpr auto p = parse(any, "anything");
  static_assert(p.has_value());
  constexpr auto s = string(p.value());
  static_assert(streq(s, "a"));
  constexpr auto p2 = parse(any, "other");
  static_assert(p2.has_value());
  static_assert(streq(string(p2.value()), "o"));
}

TEST(RangeParser, SucceedShouldConsumeNothing) {
  constexpr auto p = parse(succeed, "anything");
  static_assert(p.has_value());
  constexpr auto s = string(p.value());
  static_assert(streq(s, ""));
  constexpr auto p2 = parse(succeed, "other");
  static_assert(p2.has_value());
  static_assert(streq(string(p2.value()), ""));
}

TEST(RangeParser, CharacterShouldWork) {
  constexpr auto p = parse('a', "anything");
  static_assert(p.has_value());
  constexpr auto s = string(p.value());
  static_assert(streq(s, "a"));
  constexpr auto p2 = parse('a', "other");
  static_assert(p2.is_error());
}

TEST(RangeParser, StringShouldWork) {
  constexpr auto p = parse("any", "anything");
  static_assert(p.has_value());
  constexpr auto s = string(p.value());
  static_assert(streq(s, "any"));
  constexpr auto p2 = parse("any", "another");
  static_assert(p2.is_error());
}

TEST(RangeParser, BothShouldCombineResults) {
  auto p = parse(both{character{'a'}, static_string{"ny"}}, "anything");
  ASSERT_TRUE(p.has_value());
  ASSERT_TRUE(streq(string(p.value()), "any"));
}