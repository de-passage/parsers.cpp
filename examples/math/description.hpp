#ifndef GUARD_EXAMPLE_MATH_DEFINITION_HPP
#define GUARD_EXAMPLE_MATH_DEFINITION_HPP

#include "./ast.hpp"
#include "./functions.hpp"

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
using mult = character<'*'>;
using div = character<'/'>;
using pow = character<'^'>;
using negative_integer = map<both<d<minus>, ascii::integer>, std::negate<>>;
template <class... Ts>
using parenthesised =
    sequence<opening_parenthese, spaces, Ts..., spaces, closing_parenthese>;
struct to_ast {
  template <class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
  [[nodiscard]] ast::math_expression_ptr operator()(T i) const noexcept {
    return std::make_unique<ast::literal>(static_cast<int>(i));
  }

  template <
      class T,
      std::enable_if_t<dpsg::is_template_instance_v<T, std::tuple>, int> = 0>
  [[nodiscard]] ast::math_expression_ptr operator()(T&& tpl) const noexcept {
    return std::make_unique<
        ast::binary_operation<std::tuple_element_t<1, std::decay_t<T>>>>(
        std::get<0>(std::forward<T>(tpl)), std::get<2>(std::forward<T>(tpl)));
  }

  template <class T,
            std::enable_if_t<!std::is_integral_v<T> &&
                                 !dpsg::is_template_instance_v<T, std::tuple>,
                             int> = 0>
  [[nodiscard]] ast::math_expression_ptr operator()(T&& ptr) const noexcept {
    return static_cast<ast::math_expression_ptr>(std::forward<T>(ptr));
  }
};

using number = choose<negative_integer, ascii::integer>;
using number_ptr = map<number, to_ast>;

struct rec_math_expression;

using restricted_math_expression =
    choose<number_ptr, parenthesised<rec_math_expression>>;

template <class Sym, class Op>
using binary_operation = map<sequence<restricted_math_expression,
                                      spaces,
                                      map<Sym, always<Op>>,
                                      spaces,
                                      rec_math_expression>,
                             to_ast>;
using plus_op = binary_operation<plus, std::plus<>>;
using minus_op = binary_operation<minus, std::minus<>>;
using mult_op = binary_operation<mult, std::multiplies<>>;
using div_op = binary_operation<div, std::divides<>>;
using pow_op = binary_operation<pow, power>;
using operation = choose<plus_op, minus_op, mult_op, div_op, pow_op>;

struct rec_math_expression
    : cast<ast::math_expression_ptr,
           recursive<choose<operation,
                            number_ptr,
                            parenthesised<rec_math_expression>>>> {};

namespace detail {
template <class... Ts>
using math_seq = sequence<spaces, rec_math_expression, spaces, Ts...>;

}  // namespace detail

static_assert(is_sequence_v<detail::math_seq<>>);
static_assert(!is_dynamic_range_v<detail::math_seq<>>);
constexpr detail::math_seq<eos_t> math_expression;
constexpr detail::math_seq<> open_ended_math_expression;

}  // namespace math

#endif  // GUARD_EXAMPLE_MATH_DEFINITION_HPP