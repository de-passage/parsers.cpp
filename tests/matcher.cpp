#include <gtest/gtest.h>
#include <parsers/parsers.hpp>
#include "parsers/dsl.hpp"

#include <string>

using parsers::match;
using parsers::match_length;
using namespace parsers::dsl;
using namespace std::literals::string_literals;
using namespace parsers::description;

using eos_t = either<character<'\0'>, end_t>;
constexpr auto eos = eos_t{};

TEST(Matcher, ShouldBehaveWithTrivialDescriptors) {
  static_assert(match(any, "a"));
  static_assert(match(any, "b"));
  static_assert(match(any, ""));  // matches trailing 0
  ASSERT_TRUE(match(any, "a"s));
  ASSERT_TRUE(match(any, "b"s));
  // trailing 0 not iterated over
  ASSERT_FALSE(match(any, ""s));

  static_assert(!match(fail, "a"));
  static_assert(!match(fail, "b"));
  static_assert(!match(fail, ""));
  ASSERT_FALSE(match(fail, "a"s));
  ASSERT_FALSE(match(fail, "b"s));
  ASSERT_FALSE(match(fail, ""s));

  static_assert(!match(end, "a"));
  static_assert(!match(end, "b"));
  static_assert(!match(end, ""));  // trailing \0 prevents success
  ASSERT_FALSE(match(end, "a"s));
  ASSERT_FALSE(match(end, "b"s));
  // trailing \0 not processed when iterating over std::string
  ASSERT_TRUE(match(end, ""s));

  static_assert(match(succeed, ""));
  static_assert(match(succeed, "a"));
  static_assert(match_length(succeed, "a") == 0);
  static_assert(match_length(succeed, "") == 0);
}

TEST(Matcher, ShouldMatchSingleCharacter) {
  constexpr character<'a'> a_;

  ASSERT_TRUE(match(a_, "a"s));
  ASSERT_TRUE(match(a_, "aa"s));
  ASSERT_TRUE(match(a_, "ab"s));
  ASSERT_FALSE(match(a_, "ba"s));
  ASSERT_FALSE(match(a_, ""s));
  static_assert(match(a_, "a"));
  static_assert(match(a_, "aa"));
  static_assert(match(a_, "ab"));
  static_assert(!match(a_, "ba"));
  static_assert(!match(a_, ""));
}

TEST(Matcher, ManyShouldMatchAnything) {
  using a_ = character<'a'>;
  constexpr many<a_> many_a{};

  static_assert(match(many_a, ""));
  static_assert(match_length(many_a, "") == 0);
  static_assert(match(many_a, "a"));
  static_assert(match_length(many_a, "a") == 1);
  static_assert(match(many_a, "aaaa"));
  static_assert(match_length(many_a, "aaaa") == 4);
  static_assert(match(many_a, "b"));
  ASSERT_TRUE(match(many_a, ""s));
  ASSERT_EQ(match_length(many_a, ""s), 0);
  ASSERT_TRUE(match(many_a, "a"s));
  ASSERT_EQ(match_length(many_a, "a"s), 1);
  ASSERT_TRUE(match(many_a, "aaaa"s));
  ASSERT_EQ(match_length(many_a, "aaaa"s), 4);
  ASSERT_TRUE(match(many_a, "b"s));
}

TEST(Matcher, BothShouldMatchCorrectly) {
  constexpr both<character<'a'>, character<'b'>> ab_;

  static_assert(match(ab_, "ab"));
  static_assert(match(ab_, "abc"));
  static_assert(!match(ab_, "ba"));
  static_assert(!match(ab_, "a"));
  static_assert(!match(ab_, "b"));
  static_assert(!match(ab_, ""));

  ASSERT_TRUE(match(ab_, "ab"));
  ASSERT_TRUE(match(ab_, "abc"));
  ASSERT_FALSE(match(ab_, "ba"));
  ASSERT_FALSE(match(ab_, "a"));
  ASSERT_FALSE(match(ab_, "b"));
  ASSERT_FALSE(match(ab_, ""));
}

TEST(Matcher, EitherShouldMatchCorrectly) {
  constexpr either<character<'a'>, character<'b'>> ab_;

  static_assert(match(ab_, "ab"));
  static_assert(match(ab_, "ba"));
  static_assert(match(ab_, "a"));
  static_assert(match(ab_, "b"));
  static_assert(!match(ab_, ""));
  static_assert(!match(ab_, "c"));

  ASSERT_TRUE(match(ab_, "ab"s));
  ASSERT_TRUE(match(ab_, "b"s));
  ASSERT_FALSE(match(ab_, "c"s));
  ASSERT_FALSE(match(ab_, ""s));

  static_assert(match(eos, ""));
  ASSERT_TRUE(match(eos, ""s));
}

TEST(Matcher, RecursiveShouldMatchCorrectly) {
  struct rec_t : recursive<either<both<character<'a'>, rec_t>, eos_t>> {
  } constexpr rec;

  static_assert(match(rec, ""));
  static_assert(match(rec, "a"));
  static_assert(match(rec, "aa"));
  static_assert(match(rec, "aaa"));
  static_assert(match(rec, "aaaa"));
  static_assert(!match(rec, "b"));
}

TEST(Matcher, StaticStringShouldBehaveCorrectly) {
  constexpr auto s = "test"_s;
  static_assert(match(s, "test"));
  static_assert(match(s, "testing"));
  static_assert(!match(s, "tes"));
  ASSERT_TRUE(match(s, "test"s));
  ASSERT_TRUE(match(s, "testing"s));
  ASSERT_FALSE(match(s, "tes"s));

  constexpr auto s2 = "yrdy"_s;
  constexpr auto s_or_s2 = s | s2;
  static_assert(match(s_or_s2, "test"));
  static_assert(match(s_or_s2, "yrdy"));
  static_assert(!match(s_or_s2, "trdy"));
}

TEST(Matcher, DynamicCharactersShouldBehaveCorrectly) {
  constexpr auto c1 = character{'?'};
  constexpr auto c2 = character{'!'};
  static_assert(match(c1, "?"));
  static_assert(!match(c1, "!"));
  static_assert(!match(c2, "?"));
  static_assert(match(c2, "!"));
  static_assert(match(c1 | c2, "!"));
  static_assert(match(c1 | c2, "?"));
  static_assert(!match(c1 | c2, "#"));
  static_assert(match(c1 + c2, "?!"));
  static_assert(!match(c1 + c2, "!?"));
  ASSERT_TRUE(match(c1, "?"s));
  ASSERT_FALSE(match(c1, "!"s));
  ASSERT_FALSE(match(c2, "?"s));
  ASSERT_TRUE(match(c2, "!"s));
  ASSERT_TRUE(match(c1 | c2, "!"s));
  ASSERT_TRUE(match(c1 | c2, "?"s));
  ASSERT_FALSE(match(c1 | c2, "#"s));
  ASSERT_TRUE(match(c1 + c2, "?!"s));
  ASSERT_FALSE(match(c1 + c2, "!?"s));
}