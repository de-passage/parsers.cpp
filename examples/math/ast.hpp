#ifndef GUARD_EXAMPLE_MATH_AST_HPP
#define GUARD_EXAMPLE_MATH_AST_HPP

#include <functional>
#include <memory>

#include "./functions.hpp"

namespace math::ast {
struct math_expression;
}

namespace math::visitors {
struct interface {
  interface() noexcept = default;
  interface(interface&&) = delete;
  interface(const interface&) = delete;
  interface& operator=(interface&&) = delete;
  interface& operator=(const interface&) = delete;
  virtual void operator()(int) = 0;
  virtual void operator()(const operator_t&,
                          const ast::math_expression&,
                          const ast::math_expression&) = 0;
  virtual ~interface() = default;
};

}  // namespace math::visitors

namespace math::ast {
struct math_expression {
  math_expression() noexcept = default;
  math_expression(math_expression&&) = delete;
  math_expression(const math_expression&) = delete;
  math_expression& operator=(math_expression&&) = delete;
  math_expression& operator=(const math_expression&) = delete;

  [[nodiscard]] virtual int evaluate() const = 0;
  virtual void visit(visitors::interface& visitor) const = 0;
  virtual ~math_expression() = default;
};

using math_expression_ptr = std::unique_ptr<math_expression>;

struct literal : math_expression {
  template <class T>
  constexpr literal(T val) noexcept : value(static_cast<int>(val)) {}
  [[nodiscard]] int evaluate() const override { return value; }
  [[nodiscard]] constexpr operator int() const noexcept { return value; }

  void visit(visitors::interface& visitor) const override { visitor(value); }

 private:
  int value{};
};

template <class Op>
struct binary_operation : math_expression {
  binary_operation(math_expression_ptr&& left,
                   math_expression_ptr&& right) noexcept
      : left{std::move(left)}, right{std::move(right)} {}
  [[nodiscard]] int evaluate() const override {
    return Op{}(left->evaluate(), right->evaluate());
  }
  void visit(visitors::interface& visitor) const override {
    visitor(operator_for<Op>::value(), *left.get(), *right.get());
  }

 private:
  math_expression_ptr left;
  math_expression_ptr right;
};

}  // namespace math::ast

#endif  // GUARD_EXAMPLE_MATH_AST_HPP