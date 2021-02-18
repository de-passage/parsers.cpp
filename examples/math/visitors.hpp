#ifndef GUARD_EXAMPLES_MATH_VISITORS_HPP
#define GUARD_EXAMPLES_MATH_VISITORS_HPP

#include "./ast.hpp"

#include <iostream>
#include <string>

namespace math::visitors {
struct evaluator : interface {
  void operator()(int i) override { value = i; }
  void operator()(const operator_t& op,
                  const ast::math_expression& left,
                  const ast::math_expression& right) override {
    evaluator l_eval;
    evaluator r_eval;
    left.visit(l_eval);
    right.visit(r_eval);
    value = op(l_eval.value, r_eval.value);
  }

  int value{0};
};

template <class Format>
struct writer : interface {
  writer(std::ostream& out) noexcept : ref(out) {}
  void operator()(int i) override { ref << i; }
  void operator()(const operator_t& op,
                  const ast::math_expression& left,
                  const ast::math_expression& right) override {
    Format::fmt(
        ref,
        represent(op.description()),
        [this, &left] { left.visit(*this); },
        [this, &right] { right.visit(*this); });
  }

 private:
  std::ostream& ref;
  static std::string represent(operators op) {
    switch (op) {
      case operators::plus:
        return "+";
      case operators::minus:
        return "-";
      case operators::div:
        return "/";
      case operators::mult:
        return "*";
      case operators::pow:
        return "^";
    }
  }
};

struct lispfmt {
  static void fmt(std::ostream& out,
                  const std::string& op,
                  const std::function<void()>& write_left,
                  const std::function<void()>& write_right) {
    out << "(" << op << " ";
    write_left();
    out << " ";
    write_right();
    out << ")";
  }
};
using lisp_writer = writer<lispfmt>;

struct mathfmt {
  static void fmt(std::ostream& out,
                  const std::string& op,
                  const std::function<void()>& write_left,
                  const std::function<void()>& write_right) {
    out << "(";
    write_left();
    out << " " << op << " ";
    write_right();
    out << ")";
  }
};
using math_writer = writer<mathfmt>;

}  // namespace math::visitors

#endif  // GUARD_EXAMPLES_MATH_VISITORS_HPP