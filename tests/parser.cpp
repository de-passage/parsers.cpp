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
  constexpr auto& test = "test";
  constexpr auto c = parse('c', test);
  static_assert(c.is_error());
  constexpr auto t = parse(parsers::description::character{'t'}, test);
  static_assert(t.has_value());
  static_assert(t.value().second == 't');
}
