#include "./description.hpp"

#include <iostream>
#include <string>

template <class T>
void eval(T&& t) {
  using math::math_expression;
  using parsers::parse;
  using namespace parsers::interpreters;
  auto p1 = parse(math_expression, std::forward<T>(t));
  if (p1.has_value()) {
    std::cout << t << " = " << p1.value()->evaluate() << std::endl;
  }
  else {
    std::cout << "evaluation of \'" << t << "\' failed\n";
  }
}

int main() {
  eval("(2 + 2) - 1");
}