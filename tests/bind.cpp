#include <parsers/parsers.hpp>

#include <parsers/description/bind.hpp>

#include <gtest/gtest.h>

namespace example {
using namespace parsers::description;
using namespace parsers::dsl;

struct n_char {
  char c;
  constexpr auto operator()(int n) const noexcept {
    return at_least{static_cast<std::size_t>(n), c};
  }
};

struct to_int {
  constexpr int operator()(char c) const noexcept { return c - '0'; }
} constexpr to_int;

constexpr auto _1 = bind{map{ascii::digit, to_int}, n_char{'a'}};

}  // namespace example

TEST(Bind, ShouldWorkUnchangedWithMatcher) {
  using namespace example;
  static_assert(
      std::is_same_v<decltype(parsers::interpreters::object_parser::value(
                         _1.interpret()(std::declval<char*>(),
                                        std::declval<char*>()))),
                     int>);

  static_assert(parsers::match(_1, "3aaa"));
  static_assert(!parsers::match(_1, "3aa"));
}