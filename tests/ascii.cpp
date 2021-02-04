#include <parsers/parsers.hpp>
#include "parsers/description/char_utils.hpp"

#include <gtest/gtest.h>

TEST(Ascii, LookupTables) {
  using namespace characters::ascii;
  static_assert(to_lower('a') == 'a');
  static_assert(to_lower('A') == 'a');
  static_assert(to_lower('.') == '.');
  static_assert(to_lower('Z') == 'z');
  static_assert(to_upper('a') == 'A');
  static_assert(to_upper('b') == 'B');
  static_assert(to_upper('f') == 'F');
  static_assert(to_upper('g') == 'G');
  static_assert(to_upper('A') == 'A');
  static_assert(to_upper('.') == '.');
  static_assert(to_upper('z') == 'Z');
  static_assert(toggle_case('a') == 'A');
  static_assert(toggle_case('A') == 'a');
  static_assert(toggle_case('.') == '.');
  static_assert(toggle_case('Z') == 'z');
}

TEST(Ascii, CharacterClasses) {
  using namespace parsers::description;
  using namespace parsers::description::ascii;
  using namespace parsers::dsl;
  static_assert(parsers::match_full(many{digit}, "0123456789"_s));
  static_assert(parsers::match_full(many{space}, "   \t\n\r"_s));
  static_assert(parsers::match_full(many{alpha}, "abcdefgHIJKLMNO"_s));
  static_assert(!parsers::match(digit, "a"));
  static_assert(!parsers::match(alpha, "0"));
}