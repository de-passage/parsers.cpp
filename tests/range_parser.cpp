#include <parsers/parsers.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace parsers::dsl;
using namespace std::literals::string_literals;
using namespace parsers::description;

constexpr auto string(const std::pair<const char*, const char*>& p) noexcept {
  return parsers::range{p.first, p.second};
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
  static_assert(s == "a");
  constexpr auto p2 = parse(any, "other");
  static_assert(p2.has_value());
  static_assert(string(p2.value()) == "o");
}

TEST(RangeParser, SucceedShouldConsumeNothing) {
  constexpr auto p = parse(succeed, "anything");
  static_assert(p.has_value());
  constexpr auto s = string(p.value());
  static_assert(s == "");
  constexpr auto p2 = parse(succeed, "other");
  static_assert(p2.has_value());
  static_assert(string(p2.value()) == "");
}

TEST(RangeParser, CharacterShouldWork) {
  constexpr auto p = parse('a', "anything");
  static_assert(p.has_value());
  constexpr auto s = string(p.value());
  static_assert(s == "a");
  constexpr auto p2 = parse('a', "other");
  static_assert(p2.is_error());
}

TEST(RangeParser, StringShouldWork) {
  constexpr auto p = parse("any", "anything");
  static_assert(p.has_value());
  constexpr auto s = string(p.value());
  static_assert(s == "any");
  constexpr auto p2 = parse("any", "another");
  static_assert(p2.is_error());
}

TEST(RangeParser, BothShouldCombineResults) {
  constexpr auto d = both{character{'a'}, static_string{"ny"}};
  constexpr auto p1 = parse(d, "anything");
  constexpr auto p2 = parse(d, "nothing");

  static_assert(p1.has_value());
  static_assert(string(p1.value()) == "any");
  static_assert(p2.is_error());
}

TEST(RangeParser, EitherShouldSelectResult) {
  constexpr auto d = either{character{'H'}, static_string{"str"}};
  constexpr auto p1 = parse(d, "Hello World");
  constexpr auto p2 = parse(d, "string");
  constexpr auto p3 = parse(d, "none");
  static_assert(p1.has_value());
  static_assert(p2.has_value());
  static_assert(p3.is_error());
  static_assert(string(p1.value()) == "H");
  static_assert(string(p2.value()) == "str");
}

TEST(RangeParser, ManyShouldAggregateResults) {
  constexpr auto d = many{static_string{"ab"}};
  constexpr auto p1 = parse(d, "abababcab");
  constexpr auto p2 = parse(d, "dababab");
  static_assert(p1.has_value());
  static_assert(p2.has_value());
  static_assert(string(p1.value()) == "ababab");
  static_assert(string(p2.value()) == "");
}

TEST(RangeParser, RecursiveShouldAggregateResults) {
  struct rec_t : recursive<either<both<character<'a'>, rec_t>, eos_t>> {
  } constexpr rec;
  constexpr auto p1 = parse(rec, "aaa"_s);
  constexpr auto p2 = parse(rec, ""_s);
  static_assert(p1.has_value());
  static_assert(p2.has_value());
  static_assert(string(p1.value()) == "aaa");
  static_assert(string(p2.value()) == "");
}