#ifndef TESTS_STREQ_HPP
#define TESTS_STREQ_HPP

#include <utility>

template <class L, class R>
constexpr bool streq(L&& str1, R&& str2) noexcept {
  using std::begin;
  using std::end;
  auto beg1 = begin(str1);
  auto end1 = end(str1);
  auto beg2 = begin(str2);
  auto end2 = end(str2);
  while (true) {
    bool endof1 = (beg1 == end1 || *beg1 == '\0');
    bool endof2 = (beg2 == end2 || *beg2 == '\0');
    if (endof1 && endof2) {
      return true;
    }
    if (endof1 || endof2) {
      return false;
    }
    if (*beg1 != *beg2) {
      return false;
    }
    ++beg1;
    ++beg2;
  }
}

static_assert(!streq("test", "tes"));
static_assert(!streq("1234", "4444"));
static_assert(streq("12345", "12345"));
static_assert(!streq("tes", "test"));

#endif  // TESTS_STREQ_HPP