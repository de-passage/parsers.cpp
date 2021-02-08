#include <parsers/parsers.hpp>

#include <parsers/description/discard.hpp>

#include <gtest/gtest.h>

TEST(Discard, ShouldWorkUnchangedWithMatcher) {
  using namespace parsers::description;
  using namespace parsers::dsl;
  static_assert(parsers::match(discard{both{many{'a'}, eos}}, "aaaaa"));
  static_assert(parsers::match(sequence{discard{'a'}, 'b'}, "ab"));
}
