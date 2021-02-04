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

TEST(AdditionalDescriptions, CaseInsensitiveStrings) {
  using namespace parsers::dsl;
  using namespace parsers::description;

  constexpr auto test = "some sentence"_is;
  static_assert(parsers::match(test, "some sentence"));
  static_assert(parsers::match(test, "SOME SENTENCE"));
  static_assert(!parsers::match(test, "any sentence"));
  static_assert(parsers::match(test, "sOmE SeNtEnCe"));

  std::string s = "SomE SENTence!! trail";
  ASSERT_EQ(parsers::match_view(test, s), "SomE SENTence");
}