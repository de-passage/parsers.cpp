#include "./description.hpp"

#include <iostream>
#include <string>

template <class T>
void eval(T&& t) {
  using math::math_expression;
  using parsers::parse;
  auto p1 = parse(math_expression, std::forward<T>(t));
  if (p1.has_value()) {
    std::cout << std::forward<T>(t) << " = " << p1.value()->evaluate()
              << std::endl;
  }
  else {
    std::cout << "evaluation of \'" << std::forward<T>(t) << "\' failed\n";
  }
}

int main() {
  eval("42");
  eval("(2 + 2) - 1");
  eval("1 + 3 + (7 * 12) / 2");
  eval("2 ^ (5 + 5)");
}