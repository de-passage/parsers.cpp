#include <gtest/gtest.h>

#include <parsers/dsl.hpp>
#include <parsers/parsers.hpp>

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
  constexpr auto ab_test = a_or_b & "test";
  static_assert(match(ab_test, "atest"));
  static_assert(match(ab_test, "btest"));
  static_assert(!match(ab_test, "test"));
  ASSERT_TRUE(match(a_or_b, "a"s));
  ASSERT_TRUE(match(a_or_b, "b"s));
  ASSERT_FALSE(match(a_or_b, "c"s));
  ASSERT_TRUE(match(ab_test, "atest"s));
  ASSERT_TRUE(match(ab_test, "btest"s));
  ASSERT_FALSE(match(ab_test, "test"s));
  ASSERT_TRUE(match("Hello"_s | "World" | '!', "Hello"s));
  ASSERT_TRUE(match("Hello"_s | "World" | '!', "World"s));
  ASSERT_TRUE(match("Hello"_s | "World" | '!', "!"s));
}

TEST(Dsl, ShouldFlattenSequences) {
  using namespace parsers::description;
  using fail_t = parsers::dsl::fail_t<>;
  constexpr auto a = fail | 'a' | 'b';
  using a_t = std::decay_t<decltype(a)>;
  static_assert(std::is_same_v<a_t, alternative<fail_t, char, char>>);
  constexpr auto b = a | 'c';
  using b_t = std::decay_t<decltype(b)>;
  static_assert(std::is_same_v<b_t, alternative<fail_t, char, char, char>>);
  constexpr auto c = b | a;
  using c_t = std::decay_t<decltype(c)>;
  static_assert(std::is_same_v<
                c_t,
                alternative<fail_t, char, char, char, fail_t, char, char>>);

  constexpr auto d = fail & 'a' & 'b';
  using d_t = std::decay_t<decltype(d)>;
  static_assert(std::is_same_v<d_t, sequence<fail_t, char, char>>);
  constexpr auto e = d & 'c';
  using e_t = std::decay_t<decltype(e)>;
  static_assert(std::is_same_v<e_t, sequence<fail_t, char, char, char>>);
  constexpr auto f = e & d;
  using f_t = std::decay_t<decltype(f)>;
  static_assert(
      std::is_same_v<f_t,
                     sequence<fail_t, char, char, char, fail_t, char, char>>);
}