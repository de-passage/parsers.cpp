#include <parsers/parsers.hpp>
#include "parsers/dsl.hpp"

#include <gtest/gtest.h>

using namespace parsers::description;
using namespace ascii;

using opening_parenthese = character<'('>;
using closing_parenthese = character<')'>;
using plus = character<'+'>;
using minus = character<'-'>;
using whole_number = both<digit, many<digit>>;
using number = either<both<minus, whole_number>, whole_number>;
template <class... Ts>
using parenthesised = sequence<opening_parenthese,
                               many<whitespace>,
                               Ts...,
                               many<whitespace>,
                               closing_parenthese>;

struct math_expression;

using restricted_math_expression =
    alternative<number, parenthesised<math_expression>>;

template <class Op>
using binary_operation = sequence<restricted_math_expression,
                                  many<whitespace>,
                                  Op,
                                  many<whitespace>,
                                  math_expression>;
using plus_op = binary_operation<plus>;
using minus_op = binary_operation<minus>;
using operation = alternative<plus_op, minus_op>;

struct math_expression
    : recursive<
          alternative<operation, number, parenthesised<math_expression>>> {};

template <class... Ts>
using math_seq =
    sequence<many<whitespace>, math_expression, many<whitespace>, Ts...>;

constexpr math_seq<parsers::dsl::eos_t> math;

constexpr math_seq<> open_math;

#include <iostream>

TEST(Math, CanCompileMathExpressionParser) {
  using namespace std::literals::string_literals;
  using parsers::match;
  constexpr auto valid = [](const auto& str) { return match(math, str); };
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
}