#include "./description.hpp"

using parsers::match;

constexpr auto valid = [](const auto& str) {
  return match(math::math_expression, str);
};

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

constexpr auto check_number_parser = [](const auto& str, int v) -> bool {
  auto r = parsers::parse(math::number{}, str);
  if (!r.has_value()) {
    return false;
  }
  return static_cast<int>(r.value()) == v;
};

static_assert(check_number_parser("123", 123));
static_assert(check_number_parser("-42", -42));
static_assert(check_number_parser("-0", 0));
static_assert(!check_number_parser("", 0));
static_assert(!check_number_parser("  ", 0));
static_assert(!check_number_parser("ab", 0));
static_assert(!check_number_parser("-ab", 0));