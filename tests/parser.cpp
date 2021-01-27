#include <gtest/gtest.h>

#include <parsers/parsers.hpp>

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

TEST(Parsers, FailShouldWork) {
  constexpr auto r = parse(fail, "test");
  static_assert(r.is_error());
}

TEST(Parsers, CharacterShouldWork) {
  constexpr auto& test = "test";
  constexpr auto c = parse('c', test);
  static_assert(c.is_error());
  constexpr auto t = parse(parsers::description::character{'t'}, test);
  static_assert(t.has_value());
  static_assert(t.value().second == 't');
}