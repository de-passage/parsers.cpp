#include <parsers/parsers.hpp>

#include <parsers/description/map.hpp>

#include <gtest/gtest.h>

namespace example {
using namespace parsers::description;
using namespace parsers::dsl;

struct {
  constexpr int operator()(char c) noexcept { return c - '0'; }
} constexpr to_int;

constexpr auto _1 = map{ascii::digit, to_int};
}  // namespace example

TEST(Map, MatcherShouldWorkUnchanged) {}