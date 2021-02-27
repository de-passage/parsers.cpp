#include <parsers/parsers.hpp>

#include <gtest/gtest.h>

using namespace parsers::dsl;

template <class P>
constexpr auto string(const P& pair) noexcept {
  return parsers::range{pair.first, pair.second};
}

TEST(Alternative, CanBeParsed) {
  using namespace parsers::description;
  using parsers::parse;

  constexpr auto a = alternative{"any"_s, character{'a'}, "some"_s};
  constexpr auto p1 = parse(a, "anything");
  static_assert(p1.has_value());
  constexpr auto r1 = p1.value();
  static_assert(r1.index() == 0);
  static_assert(std::get<0>(r1) == "any");

  constexpr auto p2 = parse(a, "another");
  static_assert(p2.has_value());
  constexpr auto r2 = p2.value();
  static_assert(r2.index() == 1);
  static_assert(std::get<1>(r2) == 'a');

  constexpr auto p3 = parse(a, "something");
  static_assert(p3.has_value());
  constexpr auto r3 = p3.value();
  static_assert(r3.index() == 2);
  static_assert(std::get<2>(r3) == "some");

  constexpr auto a2 = alternative{"test"_s, many{character{'t'}}};
  auto p4 = parse(a2, "test");
  ASSERT_TRUE(p4.has_value());
  const auto& r4 = p4.value();
  ASSERT_EQ(r4.index(), 0);
  ASSERT_TRUE(std::get<0>(r4) == "test");

  auto p5 = parse(a2, "tttest");
  ASSERT_TRUE(p5.has_value());
  const auto& r5 = p5.value();
  ASSERT_EQ(r5.index(), 1);
  ASSERT_EQ(std::get<1>(r5).size(), 3);
  for (std::size_t s = 0; s < 3; ++s) {
    ASSERT_EQ(std::get<1>(r5)[s], 't');
  }
}

TEST(Alternative, CanParseRanges) {
  using parsers::parse_range;
  using parsers::description::alternative;
  using parsers::description::many;
  using parsers::description::sequence;

  constexpr auto a = alternative{"any"_s, sequence{"thing"_s, many{"thing"_s}}};
  constexpr auto p1 = parse_range(a, "anything");
  static_assert(p1.has_value());
  static_assert(string(p1.value()) == "any");

  constexpr auto p2 = parse_range(a, "thingthingthing stop");
  static_assert(p2.has_value());
  static_assert(string(p2.value()) == "thingthingthing");

  constexpr auto p3 = parse_range(a, "whatever");
  static_assert(p3.is_error());
}

TEST(Alternative, MatchesCorrectly) {
  using parsers::match_length;
  using namespace parsers::description;
  constexpr auto a = alternative{character{1}, character{3}, character{5}};
  constexpr int s1[] = {1, 2, 4, 5};
  constexpr int s2[] = {2, 1, 3};
  constexpr int s3[] = {5, 1, 3, 1, 1};

  constexpr auto p1 = match_length(a, s1);
  static_assert(p1 == 1);
  constexpr auto p2 = match_length(a, s2);
  static_assert(p2 == 0);
  constexpr auto p3 = match_length(many{a}, s3);
  static_assert(p3 == 5);
}