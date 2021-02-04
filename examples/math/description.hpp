#ifndef GUARD_EXAMPLE_MATH_DEFINITION_HPP
#define GUARD_EXAMPLE_MATH_DEFINITION_HPP

#include <parsers/parsers.hpp>

namespace math {
using namespace parsers::description;
using namespace ascii;

using opening_parenthese = character<'('>;
using closing_parenthese = character<')'>;
using plus = character<'+'>;
using minus = character<'-'>;
using whole_number = many1<digit_t>;
using number = either<both<minus, whole_number>, whole_number>;
template <class... Ts>
using parenthesised = sequence<opening_parenthese,
                               many<space_t>,
                               Ts...,
                               many<space_t>,
                               closing_parenthese>;

struct rec_math_expression;

using restricted_math_expression =
    alternative<number, parenthesised<rec_math_expression>>;

template <class Op>
using binary_operation = sequence<restricted_math_expression,
                                  many<space_t>,
                                  Op,
                                  many<space_t>,
                                  rec_math_expression>;
using plus_op = binary_operation<plus>;
using minus_op = binary_operation<minus>;
using operation = alternative<plus_op, minus_op>;

struct rec_math_expression
    : recursive<
          alternative<operation, number, parenthesised<rec_math_expression>>> {
};

namespace detail {
template <class... Ts>
using math_seq =
    sequence<many<space_t>, rec_math_expression, many<space_t>, Ts...>;
}

static_assert(is_sequence_v<detail::math_seq<>>);
static_assert(!is_dynamic_range_v<detail::math_seq<>>);
constexpr detail::math_seq<parsers::dsl::eos_t> math_expression;
constexpr detail::math_seq<> open_ended_math_expression;

}  // namespace math

#endif  // GUARD_EXAMPLE_MATH_DEFINITION_HPP