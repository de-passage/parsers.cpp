#include <gtest/gtest.h>
#include <parsers/parsers.hpp>

#include <string>
#include "parsers/description.hpp"

const std::string empty{};
const std::string a{"a"};
const std::string aa{"aa"};
const std::string ab{"ab"};
const std::string ba{"ba"};

TEST(Matcher, ShouldBehaveWithTrivialDescriptors) {
  using parsers::match;
  using parsers::description::any;
  using parsers::description::end;
  using parsers::description::fail;

  static_assert(match(any{}, "a"));
  static_assert(match(any{}, "b"));
  static_assert(!match(any{}, ""));

  static_assert(!match(fail{}, "a"));
  static_assert(!match(fail{}, "b"));
  static_assert(!match(fail{}, ""));

  static_assert(!match(end{}, "a"));
  static_assert(!match(end{}, "b"));
  static_assert(match(end{}, ""));
}

TEST(Matcher, ShouldMatchSingleCharacter) {
  constexpr parsers::description::character<'a'> a_;
  using parsers::match;

  ASSERT_TRUE(match(a_, a));
  ASSERT_TRUE(match(a_, aa));
  ASSERT_TRUE(match(a_, ab));
  ASSERT_TRUE(!match(a_, ba));
  ASSERT_TRUE(!match(a_, empty));
}