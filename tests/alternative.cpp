#include <parsers/parsers.hpp>

#include <gtest/gtest.h>

#include "./streq.hpp"

using namespace parsers::dsl;

TEST(Alternative, CanBeParsed) {
  using namespace parsers::description;
  using parsers::parse;

  constexpr auto a = alternative{"any"_s, character{'a'}, "some"_s};
  constexpr auto p1 = parse(a, "anything");
  static_assert(p1.has_value());
  constexpr auto r1 = p1.value();
  static_assert(r1.index() == 0);
  static_assert(streq(std::get<0>(r1), "any"));

  constexpr auto p2 = parse(a, "another");
  static_assert(p2.has_value());
  constexpr auto r2 = p2.value();
  static_assert(r2.index() == 1);
  static_assert(std::get<1>(r2) == 'a');

  constexpr auto p3 = parse(a, "something");
  static_assert(p3.has_value());
  constexpr auto r3 = p3.value();
  static_assert(r3.index() == 2);
  static_assert(streq(std::get<2>(r3), "some"));
}