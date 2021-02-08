#include <parsers/parsers.hpp>

#include <parsers/description/discard.hpp>

#include <gtest/gtest.h>

#include "./streq.hpp"

namespace examples {
using namespace parsers::description;
using namespace parsers::dsl;
constexpr auto _1 = discard{both{many{'a'}, eos}};
constexpr auto _2 = sequence{discard{'a'}, 'b'};
constexpr auto _3 = alternative{discard{sequence{"a", eos}},
                                sequence{discard{'b'}, alternative{'c', 'd'}}};
}  // namespace examples

TEST(Discard, ShouldWorkUnchangedWithMatcher) {
  using namespace examples;
  static_assert(parsers::match_full(_1, "aaaaa"));
  static_assert(parsers::match(_2, "ab"));
  static_assert(parsers::match(_3, "a"));
}

template <class P>
constexpr std::string_view string(const P& p) noexcept {
  return std::string_view(p.first, std::distance(p.first, p.second));
}

TEST(Discard, ShouldWorkUnchangedWithRange) {
  using namespace examples;
  constexpr auto p1f = parsers::parse_range(_1, "aaabbc");
  static_assert(!p1f.has_value());
  constexpr auto p1 = parsers::parse_range(_1, "aaa");
  static_assert(p1.has_value());
  static_assert(streq(string(p1.value()), "aaa"));
  constexpr auto p2 = parsers::parse_range(_2, "ab");
  static_assert(p2.has_value());
  static_assert(streq(string(p1.value()), "ab"));
}

TEST(Discard, ShouldDiscardValuesWithObjectParser) {
  using namespace examples;
  constexpr auto p1f = parsers::parse(_1, "aaaabc");
  static_assert(!p1f.has_value());
  constexpr auto p1 = parsers::parse(_1, "aaa");
  static_assert(p1.has_value());
  static_assert(
      std::is_same_v<std::decay_t<decltype(p1.value())>, parsers::empty>);
  constexpr auto p2 = parsers::parse(_2, "ab");
  static_assert(p2.has_value());
  static_assert(std::is_same_v<std::decay_t<decltype(p2.value())>, char>);
  static_assert(p2.value() == 'b');
}