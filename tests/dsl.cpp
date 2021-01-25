#include <gtest/gtest.h>

#include <parsers/dsl.hpp>
#include <parsers/parsers.hpp>
#include "parsers/matcher.hpp"

#include <string>

using namespace parsers::dsl;
using namespace std::literals::string_literals;
using parsers::match;

TEST(Dsl, ShouldHandleCharacters) {
  static_assert(match('c', "c"));
  static_assert(match(L'd', L"d"));
  static_assert(!match('d', "c"));
  static_assert(!match(L'c', L"d"));
  ASSERT_TRUE(match('c', "c"s));
  ASSERT_TRUE(match(L'd', L"d"s));
  ASSERT_FALSE(match('d', "c"s));
  ASSERT_FALSE(match(L'c', L"d"s));
}

TEST(Dsl, ShouldHandleLiteralStrings) {
  static_assert(match("test", "test"));
  static_assert(match(L"test", L"test"));
  static_assert(!match("test", "not test"));
  static_assert(!match(L"test", L"not test"));
  ASSERT_TRUE(match("test", "test"s));
  ASSERT_TRUE(match(L"test", L"test"s));
  ASSERT_FALSE(match("test", "not test"s));
  ASSERT_FALSE(match(L"test", L"not test"s));
}

TEST(Dsl, ShouldHandleCompoundLiterals) {
  constexpr auto a_or_b = fail | 'a' | 'b';
  static_assert(match(a_or_b, "a"));
  static_assert(match(a_or_b, "b"));
  static_assert(!match(a_or_b, "c"));
  constexpr auto ab_test = a_or_b + "test";
  static_assert(match(ab_test, "atest"));
  static_assert(match(ab_test, "btest"));
  static_assert(!match(ab_test, "test"));
  ASSERT_TRUE(match(a_or_b, "a"));
  ASSERT_TRUE(match(a_or_b, "b"));
  ASSERT_FALSE(match(a_or_b, "c"));
  ASSERT_TRUE(match(ab_test, "atest"));
  ASSERT_TRUE(match(ab_test, "btest"));
  ASSERT_FALSE(match(ab_test, "test"));
  ASSERT_TRUE(match(fail | "Hello" | "World!", "Hello"));
}