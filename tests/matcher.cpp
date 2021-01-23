#include <gtest/gtest.h>
#include <parsers/parsers.hpp>

#include <string>
#include "parsers/description.hpp"

using parsers::match;
using namespace parsers::description;
constexpr auto eos = character<'\0'>{};

const std::string empty{};
const std::string a{"a"};
const std::string b{"b"};
const std::string aa{"aa"};
const std::string ab{"ab"};
const std::string ba{"ba"};

TEST(Matcher, ShouldBehaveWithTrivialDescriptors) {
  static_assert(match(any{}, "a"));
  static_assert(match(any{}, "b"));
  static_assert(match(any{}, ""));  // matches trailing 0
  ASSERT_TRUE(match(any{}, a));
  ASSERT_TRUE(match(any{}, b));
  // trailing 0 not iterated over
  ASSERT_FALSE(match(any{}, empty));

  static_assert(!match(fail{}, "a"));
  static_assert(!match(fail{}, "b"));
  static_assert(!match(fail{}, ""));
  ASSERT_FALSE(match(fail{}, a));
  ASSERT_FALSE(match(fail{}, b));
  ASSERT_FALSE(match(fail{}, empty));

  static_assert(!match(end{}, "a"));
  static_assert(!match(end{}, "b"));
  static_assert(!match(end{}, ""));  // trailing \0 prevents success
  ASSERT_FALSE(match(end{}, a));
  ASSERT_FALSE(match(end{}, b));
  // trailing \0 not processed when iterating over std::string
  ASSERT_TRUE(match(end{}, empty));
}

TEST(Matcher, ShouldMatchSingleCharacter) {
  constexpr character<'a'> a_;

  ASSERT_TRUE(match(a_, a));
  ASSERT_TRUE(match(a_, aa));
  ASSERT_TRUE(match(a_, ab));
  ASSERT_FALSE(match(a_, ba));
  ASSERT_FALSE(match(a_, empty));
  static_assert(match(a_, "a"));
  static_assert(match(a_, "aa"));
  static_assert(match(a_, "ab"));
  static_assert(!match(a_, "ba"));
  static_assert(!match(a_, ""));
  static_assert(match(eos, ""));
  ASSERT_FALSE(match(eos, empty));
}

TEST(Matcher, ManyShouldMatchAnything) {
  using a_ = character<'a'>;
  constexpr many<a_> many_a{};

  static_assert(match(many_a, ""));
  static_assert(match(many_a, "a"));
  static_assert(match(many_a, "aaaa"));
  static_assert(match(many_a, "b"));
}