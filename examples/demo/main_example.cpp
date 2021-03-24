#include <parsers/parsers.hpp>

#include <iostream>

using namespace parsers::description;
using namespace parsers::dsl;

// turns an arbitrary range (not necessarily ending with 0, as atoi requires)
// into an integer
constexpr auto to_int = [](auto beg, auto end) {
  int count = 0;
  while (beg != end) {
    count = count * 10 + *beg - '0';
    ++beg;
  }
  return count;
};

// parse a sequence of 1 or more digits into a C++ int
constexpr auto integer = many1{ascii::digit} /= to_int;

// parse an integer followed by '+' (discarded), followed by an integer, into a
// std::tuple<int, int>
constexpr auto addition = integer & ~('+'_c) & integer;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Missing input\n";
    return 1;
  }

  auto result = parsers::parse(addition, parsers::range(argv[1]));
  if (result.has_value()) {
    std::cout << std::get<0>(result.value()) + std::get<1>(result.value())
              << '\n';
  }
  else {
    std::cerr << "Invalid input\n";
    return 1;
  }

  return 0;
}