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

  constexpr auto test = "some.?\tsentence"_is;
  static_assert(parsers::match(test, "some.?\tsentence"));
  static_assert(parsers::match(test, "SOME.?\tSENTENCE"));
  static_assert(!parsers::match(test, "any.?\tsentence"));
  static_assert(parsers::match(test, "sOmE.?\tSeNtEnCe"));

  std::string s = "SomE.?\tSENTence!! trail";
  ASSERT_EQ(parsers::match_view(test, s), "SomE.?\tSENTence");
}

TEST(AdditionalDescriptions, CaseInsensitiveCharacters) {
  using namespace parsers::dsl;
  using namespace parsers::description;

  static_assert(parsers::match('a'_ic, "a"));
  static_assert(parsers::match('a'_ic, "A"));
  static_assert(!parsers::match('a'_ic, "b"));

  using many_a = many<ascii::case_insensitive_character<'a'>>;
  std::string str = "aAaaAAb";
  auto r = parsers::parse(many_a{}, str);
  ASSERT_TRUE(r.has_value());
  auto& v = r.value();
  ASSERT_EQ(v.size(), 6);
  for (int i = 0; i < 6; ++i) {
    if (i == 1 || i >= 4) {
      ASSERT_EQ(v[i], 'A');
    }
    else {
      ASSERT_EQ(v[i], 'a');
    }
  }
}