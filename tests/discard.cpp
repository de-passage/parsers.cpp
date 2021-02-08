#include <parsers/parsers.hpp>

#include <parsers/description/discard.hpp>

#include <gtest/gtest.h>

#include "./streq.hpp"

TEST(Discard, ShouldWorkUnchangedWithMatcher) {
  using namespace parsers::description;
  using namespace parsers::dsl;
  static_assert(parsers::match_full(discard{both{many{'a'}, eos}}, "aaaaa"));
  static_assert(parsers::match(sequence{discard{'a'}, 'b'}, "ab"));
  static_assert(
      parsers::match(alternative{discard{sequence{"a", eos}},
                                 sequence{discard{'b'}, alternative{'c', 'd'}}},
                     "a"));
}

template <class P>
constexpr std::string_view string(const P& p) noexcept {
  return std::string_view(p.first, std::distance(p.first, p.second));
}

TEST(Discard, ShouldWorkUnchangedWithRange) {
  using namespace parsers::description;
  using namespace parsers::dsl;
  constexpr auto p1 =
      parsers::parse_range(discard{both{many{'a'}, 'b'}}, "aaabbc");
  static_assert(p1.has_value());
  static_assert(streq(string(p1.value()), "aaab"));
}
