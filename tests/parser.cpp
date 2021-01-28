#include <gtest/gtest.h>

#include <parsers/parsers.hpp>

#include <string>
using namespace std::literals::string_literals;

template <class D, class T>
[[nodiscard]] constexpr auto parse(D&& descriptor, T&& t) noexcept {
  const auto parser =
      parsers::interpreters::make_parser<parsers::interpreters::object_parser>(
          descriptor);
  using namespace std;
  auto b = begin(t);
  auto e = end(t);
  return parser(b, e);
}

using namespace parsers::dsl;

TEST(Parsers, TrivialParsersShouldWork) {
  constexpr auto r = parse(fail, "test");
  static_assert(r.is_error());
  constexpr auto s = parse(succeed, "test");
  static_assert(s.has_value());
  constexpr auto e1 = parse(end, "test");
  static_assert(e1.is_error());
  ASSERT_TRUE(parse(end, ""s).has_value());
  constexpr auto a = parse(any, "test");
  static_assert(a.has_value());
  static_assert(a.value().second == 't');
}

TEST(Parsers, CharacterShouldWork) {
  constexpr const auto& test = "test";
  constexpr auto c = parse('c', test);
  static_assert(c.is_error());
  constexpr auto t = parse(parsers::description::character{'t'}, test);
  static_assert(t.has_value());
  static_assert(t.value().second == 't');
}

using parsers::description::static_string;
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
    if (endof1 != endof2) {
      return false;
    }
    if (endof1 == endof2) {
      return true;
    }
    if (*beg1 != *beg2) {
      return false;
    }
  }
}

TEST(Parsers, StringShouldWork) {
  constexpr const auto& test = "test";
  constexpr auto s = parse(test, "test string");
  static_assert(s.has_value());
  constexpr auto str = s.value().second;
  static_assert(streq(str, test));
}