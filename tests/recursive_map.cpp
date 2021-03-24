#include <gtest/gtest.h>

#include <parsers/parsers.hpp>

using namespace parsers::dsl;
using namespace parsers::description;

struct length {
  template <class It>
  constexpr std::ptrdiff_t operator()(It beg, It end) const noexcept {
    return std::distance(beg, end);
  }
};

struct to_int {
  constexpr int operator()([[maybe_unused]] parsers::empty) const noexcept {
    return 0;
  }
};

struct count_a
    : recursive<choose<build<both<character<'a'>, discard<count_a>>, length>,
                       map<succeed_t, to_int>>> {
} constexpr count_a;

template <class D, class T>
constexpr std::ptrdiff_t check_length(D&& d, const T& input) noexcept {
  auto r = parsers::parse(std::forward<D>(d), input);
  if (r.is_error()) {
    return -1;
  }
  return r.value();
}

TEST(RecursiveMap, ShouldCollapseIntegers) {
  ASSERT_EQ(check_length(count_a, "aaa"), 3);
  ASSERT_EQ(check_length(count_a, "aaaaaa"), 6);
  ASSERT_EQ(check_length(count_a, "b"), 0);
  ASSERT_EQ(check_length(count_a, ""), 0);
  static_assert(check_length(count_a, "aab") == 2);
}