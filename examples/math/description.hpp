#ifndef GUARD_EXAMPLE_MATH_DEFINITION_HPP
#define GUARD_EXAMPLE_MATH_DEFINITION_HPP

#include <parsers/parsers.hpp>

namespace math {
using namespace parsers::description;
using namespace ascii;

template <class T>
using d = discard<T>;
struct to_int {
  template <class T, class U>
  constexpr int operator()(T beg, const U& end) const noexcept {
    int acc = 0;
    while (beg != end) {
      acc = acc * 10 + (*beg - '0');
    }
    return acc;
  }
  constexpr int operator()(std::variant<int, int>&& var) const noexcept {
    if (var.index() == 0) {
      return std::get<0>(var) * -1;
    }
    return std::get<1>(var);
  }
};

using opening_parenthese = d<character<'('>>;
using closing_parenthese = d<character<')'>>;
using plus = character<'+'>;
using minus = character<'-'>;
using whole_number = build<many1<digit_t>, to_int>;
using number = either<both<d<minus>, whole_number>, whole_number>;
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