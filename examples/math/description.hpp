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
struct to_int {
  template <class T, class U>
  constexpr int operator()(T beg, const U& end) const noexcept {
    int acc = 0;
    while (beg != end) {
      acc = acc * 10 + (*beg - '0');
      ++beg;
    }
    return acc;
  }
  template <
      class V,
      std::enable_if_t<std::is_same_v<std::decay_t<V>, std::variant<int, int>>,
                       int> = 0>
  constexpr int operator()(V&& var) const noexcept {
    if (var.index() == 0) {
      return std::get<0>(var) * -1;
    }
    return std::get<1>(var);
  }
};
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
using whole_number = build<many1<digit_t>, to_int>;
using number = map<either<both<d<minus>, whole_number>, whole_number>, to_int>;
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