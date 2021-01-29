#include <parsers/parsers.hpp>

#include <gtest/gtest.h>
#include <string>

#include "./streq.hpp"

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
TEST(Parsers, StringShouldWork) {
  constexpr const auto& test = "test";
  constexpr auto s = parse(test, "test string");
  static_assert(s.has_value());
  constexpr auto str = s.value().second;
  static_assert(streq(str, test));
}

TEST(Parsers, ManyShouldWork) {
  using parsers::description::character;
  using parsers::description::many;
  const auto p = parse(many{character<'a'>{}}, "aaaab");
  ASSERT_TRUE(p.has_value());
  const auto& r = p.value().second;
  ASSERT_EQ(r.size(), 4);
  for (std::size_t s = 0; s < r.size(); ++s) {
    ASSERT_EQ(r[s], 'a');
  }
}

TEST(Parsers, EitherShouldWork) {
  using namespace parsers::description;
  constexpr auto d = either{character<'a'>{}, "te"_s};
  constexpr auto p = parse(d, "test");
  static_assert(p.has_value());
  constexpr auto e = p.value().second;
  static_assert(e.index() == 1);
  static_assert(streq(std::get<1>(e), "te"));
  static_assert(parse(d, "nope").is_error());
}

TEST(Parsers, BothShouldWork) {
  using namespace parsers::description;
  constexpr auto d = both{character<'H'>{}, "ello"_s};
  constexpr auto p = parse(d, "Hello World!");
  static_assert(p.has_value());
  constexpr auto b = p.value().second;
  static_assert(std::get<0>(b) == 'H');
  static_assert(streq(std::get<1>(b), "ello"));
  static_assert(parse(d, "Hi there").is_error());
}