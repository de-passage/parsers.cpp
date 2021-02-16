#include "./description.hpp"

#include <iostream>
#include <string>

using namespace std::literals::string_literals;
using parsers::match;

constexpr auto valid = [](const auto& str) {
  return match(math::math_expression, str);
};
static_assert(!match(math::whole_number(), ""));
static_assert(!valid(""));
static_assert(!valid(" "));
static_assert(!valid("()"));
static_assert(valid("  -42 "));
static_assert(valid("18"));
static_assert(valid("-18"));
static_assert(valid("(1)"));
static_assert(valid("(-42)"));
static_assert(valid("1+1"));
static_assert(valid("-32 + 14"));
static_assert(valid("( 43 + -8) + 14"));
static_assert(valid("1+1--2"));
static_assert(!valid("1 + ( 1 + ( 2 - 1)"));
static_assert(!valid("1 + 2)"));

int main() {}