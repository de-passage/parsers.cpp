#include <parsers/parsers.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace std::literals::string_literals;

template <class D, class T>
[[nodiscard]] constexpr auto parse(D&& descriptor, T&& t) noexcept {
  const auto parser =
      parsers::interpreters::make_parser<parsers::interpreters::object_parser>(
          descriptor);
  using std::begin;
  using std::end;
  auto b = begin(t);
  auto e = end(t);
  return parser(b, e);
}

using namespace parsers::dsl;

TEST(ObjectParser, TrivialParsersShouldWork) {
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

TEST(ObjectParser, CharacterShouldWork) {
  constexpr const auto& test = "test";
  constexpr auto c = parse('c', test);
  static_assert(c.is_error());
  constexpr auto t = parse(parsers::description::character{'t'}, test);
  static_assert(t.has_value());
  static_assert(t.value().second == 't');
}

using parsers::description::static_string;
TEST(ObjectParser, StringShouldWork) {
  constexpr const auto& test = "test";
  constexpr auto s = parse(test, "test string");
  static_assert(s.has_value());
  constexpr auto str = s.value().second;
  static_assert(str == test);
}

TEST(ObjectParser, ManyShouldWork) {
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

TEST(ObjectParser, EitherShouldWork) {
  using namespace parsers::description;
  constexpr auto d = either{character<'a'>{}, "te"_s};
  constexpr auto p = parse(d, "test");
  static_assert(p.has_value());
  constexpr auto e = p.value().second;
  static_assert(e.index() == 1);
  static_assert(std::get<1>(e) == "te");
  static_assert(parse(d, "nope").is_error());
}

TEST(ObjectParser, BothShouldWork) {
  using namespace parsers::description;
  constexpr auto d = both{character<'H'>{}, "ello"_s};
  constexpr auto p = parse(d, "Hello World!");
  static_assert(p.has_value());
  constexpr auto b = p.value().second;
  static_assert(std::get<0>(b) == 'H');
  static_assert(std::get<1>(b) == "ello");
  static_assert(parse(d, "Hi there").is_error());
}

TEST(ObjectParser, RecursiveShouldWork) {
  using namespace parsers::description;
  struct rec_t : recursive<either<both<character<'a'>, rec_t>,
                                  either<character<'\0'>, end_t>>> {
  } constexpr rec;
  auto p = parse(rec, "aa");
  ASSERT_TRUE(p.has_value());
  ASSERT_EQ(p.value().second.index(), 0);
  auto& l = std::get<0>(p.value().second);
  ASSERT_EQ(std::get<0>(l), 'a');
  ASSERT_TRUE(std::get<1>(l));
  auto& ls = *std::get<1>(l);
  ASSERT_EQ(ls.index(), 0);
  auto& lsl = std::get<0>(ls);
  ASSERT_EQ(std::get<0>(lsl), 'a');
  ASSERT_TRUE(std::get<1>(lsl));
  auto& lsls = *std::get<1>(lsl);
  ASSERT_EQ(lsls.index(), 1);
  ASSERT_EQ(std::get<1>(lsls).index(), 0);

  // testing for accidental copies
  using namespace parsers::interpreters;

  using indir = parsers::interpreters::detail::
      unique_ptr<rec_t, parsers::interpreters::object_parser, const char*>;
  using var = std::variant<std::tuple<char,
                                      parsers::interpreters::detail::unique_ptr<
                                          rec_t,
                                          parsers::interpreters::object_parser,
                                          const char*>>,
                           std::variant<char, parsers::empty>>;

  static_assert(std::is_same_v<
                parsers::interpreters::detail::
                    recursive_pointer_type<rec_t, object_parser, const char*>,
                var>);
  static_assert(std::is_constructible_v<indir, var&&>);
  // static_assert(std::is_convertible_v<var&&, indir>);

  static_assert(!std::is_copy_constructible_v<indir>);
}