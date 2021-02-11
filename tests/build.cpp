#include <parsers/parsers.hpp>

#include <parsers/description/build.hpp>

#include <gtest/gtest.h>

namespace examples {
using namespace parsers::description;
using namespace parsers::dsl;
struct length {
  template <class I>
  constexpr std::size_t operator()(I beg, I end) const noexcept {
    return static_cast<std::size_t>(end - beg);
  }
} constexpr static length;
constexpr auto _1 = build{sequence{many{'a'}, eos}, length};
}  // namespace examples

TEST(Build, MatcherShouldWorkUnchanged) {
  using namespace examples;
  static_assert(parsers::match(_1, "aaa"));
  static_assert(!parsers::match(_1, "aab"));
}