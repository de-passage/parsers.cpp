#include <iostream>
#include <utility>

#include "./description.hpp"

int main() {
  std::cout << "result: "
            << parsers::match_view(
                   cli_options::option_list{},
                   " -ptest --option \"something \" -a --again = true")
            << std::endl;
  return 0;
}