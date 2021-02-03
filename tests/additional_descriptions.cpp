#include <parsers/parsers.hpp>

#include <gtest/gtest.h>

TEST(AdditionalDescriptions, Many1) {
  using namespace parsers::description;
  using many1_a = many1<character<'a'>>;
  static_assert(parsers::match(many1_a{}, "aaaa"));
  static_assert(!parsers::match(many1_a{}, ""));

  auto r = parsers::parse(many1_a{}, "aaaa");
  ASSERT_TRUE(r.has_value());
  ASSERT_EQ(r.value().size(), 4);
}