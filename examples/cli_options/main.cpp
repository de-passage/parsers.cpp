#include <iostream>

#include "./description.hpp"
#include "parsers/parsers.hpp"

constexpr auto option_list = [] {
  using namespace parsers::dsl;
  return cli_options::option_list{} + parsers::dsl::eos;
}();

constexpr auto valid = [](const auto& str) {
  return parsers::match_full(option_list, str);
};

static_assert(valid(""));
static_assert(valid("  "));
static_assert(valid(" -p "));
static_assert(valid(" --option "));
static_assert(valid(" -t true "));
static_assert(valid(" -ttrue "));
static_assert(valid(" --option = \"see me\" "));
static_assert(!valid(" --option = \" unterminated "));
static_assert(!valid(" -p\" "));
static_assert(valid(" -p -a"));
static_assert(valid(" -ptest --option \"something \" -a --again = true"));
static_assert(valid(" -p\"\\\"\" "));

int main() {
  std::cout << "result: "
            << parsers::match_view(option_list, " --option = \"see\" ")
            << std::endl;
  return 0;
}