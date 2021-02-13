#include <parsers/parsers.hpp>

#include <parsers/description/build.hpp>

#include <gtest/gtest.h>

#include "./streq.hpp"

namespace examples {
using namespace parsers::description;
using namespace parsers::dsl;
struct length {
  template <class I>
  constexpr std::size_t operator()(I beg, I end) const noexcept {
    return static_cast<std::size_t>(end - beg);
  }
} constexpr static length;
struct {
  template <class I>
  constexpr auto operator()(I beg, I end) const noexcept {
    return static_string{beg, end};
  }
} constexpr static to_string;

constexpr auto _1 = build{sequence{many{'a'}, eos}, length};
constexpr auto _2 =
    build{sequence{either{'a', 'b'}, either{"string", 'c'}}, to_string};
}  // namespace examples

TEST(Build, MatcherShouldWorkUnchanged) {
  using namespace examples;
  static_assert(parsers::match(_1, "aaa"));
  static_assert(!parsers::match(_1, "aab"));
  static_assert(parsers::match(_2, "bc"));
  static_assert(parsers::match(_2, "ac"));
  static_assert(parsers::match(_2, "astring"));
  static_assert(parsers::match(_2, "bstring"));
  static_assert(!parsers::match(_2, "cstring"));
  // static_assert(parsers::match(both{many{_2}, eos}, "abastring"));
}

template <class T, class U>
constexpr bool check(T&& pair, U&& e) noexcept {
  return streq(parsers::description::static_string(pair.first, pair.second),
               std::forward<U>(e));
}

TEST(Build, RangeParserShouldWorkUnchanged) {
  using namespace examples;
  constexpr auto p1 = parsers::parse_range(_1, "aaa");
  static_assert(p1.has_value());
  static_assert(check(p1.value(), "aaa"));
  static_assert(!parsers::parse_range(_1, "aaab").has_value());
  constexpr auto p2 = parsers::parse_range(_2, "ace");
  static_assert(p2.has_value());
  static_assert(check(p2.value(), "ac"));
}

TEST(Build, ObjectParserShouldReturnObjectFromRange) {
  using namespace examples;
  constexpr auto p1 = parsers::parse(_1, "aaa");
  static_assert(p1.has_value());
  static_assert(p1.value() == 4);
  constexpr auto p2 = parsers::parse(_1, "aaaaa");
  static_assert(p2.value() == 6);
  constexpr auto p3 = parsers::parse(_2, "astring rest");
  static_assert(p3.has_value());
  static_assert(streq(p3.value(), "astring"));
}