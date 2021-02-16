#ifndef GUARD_EXAMPLE_MATH_DEFINITION_HPP
#define GUARD_EXAMPLE_MATH_DEFINITION_HPP

#include <functional>
#include <parsers/parsers.hpp>
#include <type_traits>

namespace math {
using namespace parsers::description;
using namespace ascii;

template <class T>
using d = discard<T>;
template <class T>
struct always {
  template <class... Ts>
  constexpr T operator()([[maybe_unused]] Ts&&...) const noexcept {
    return T{};
  }
};

using opening_parenthese = d<character<'('>>;
using closing_parenthese = d<character<')'>>;
using spaces = d<many<space_t>>;
using plus = character<'+'>;
using minus = character<'-'>;
using whole_number = ascii::integer;
using negative_wnumber = map<both<d<minus>, whole_number>, std::negate<>>;
using number = choose<negative_wnumber, whole_number>;
template <class... Ts>
using parenthesised =
    sequence<opening_parenthese, spaces, Ts..., spaces, closing_parenthese>;

struct rec_math_expression;

using restricted_math_expression =
    alternative<number, parenthesised<rec_math_expression>>;

template <class Sym, class Op>
using binary_operation = sequence<restricted_math_expression,
                                  spaces,
                                  map<Sym, always<Op>>,
                                  spaces,
                                  rec_math_expression>;
using plus_op = binary_operation<plus, std::plus<>>;
using minus_op = binary_operation<minus, std::minus<>>;
using operation = alternative<plus_op, minus_op>;

struct rec_math_expression
    : recursive<
          alternative<operation, number, parenthesised<rec_math_expression>>> {
};

namespace detail {
template <class... Ts>
using math_seq = sequence<spaces, rec_math_expression, spaces, Ts...>;
}

static_assert(is_sequence_v<detail::math_seq<>>);
static_assert(!is_dynamic_range_v<detail::math_seq<>>);
constexpr detail::math_seq<parsers::dsl::eos_t> math_expression;
constexpr detail::math_seq<> open_ended_math_expression;

}  // namespace math

#endif  // GUARD_EXAMPLE_MATH_DEFINITION_HPP