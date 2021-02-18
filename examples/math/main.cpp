#include "./description.hpp"
#include "./visitors.hpp"

#include <iostream>
#include <string>

template <class T>
int eval(T&& t) {
  using math::math_expression;
  using parsers::parse;
  auto p1 = parse(math_expression, std::forward<T>(t));
  if (p1.has_value()) {
    auto& value = *p1.value().get();
    std::cout << "successfully parsed '" << t << "'\n";
    std::cout << "embedded evaluator:\t";
    std::cout << value.evaluate() << "\n";
    std::cout << "visitor evaluator:\t";
    math::visitors::evaluator eval;
    value.visit(eval);
    std::cout << eval.value;
    std::cout << "\nlisp writer:\t";
    math::visitors::lisp_writer lwr{std::cout};
    value.visit(lwr);
    std::cout << "\nmath writer:\t";
    math::visitors::math_writer mwr{std::cout};
    value.visit(mwr);
    std::cout << '\n' << std::endl;

    return 0;
  }
  std::cout << "evaluation of \'" << t << "\' failed\n";
  return 1;
}

int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: math_parser \"<EXPRESSION>\"";
    return 1;
  }

  std::string acc(argv[1]);
  for (int i = 2; i < argc; ++i) {
    acc.append(argv[i]);
  }

  return eval(acc);
}