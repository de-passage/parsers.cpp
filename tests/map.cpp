#include <parsers/parsers.hpp>

#include <parsers/description/discard.hpp>
#include <parsers/description/map.hpp>
#include <type_traits>

#include <gtest/gtest.h>

namespace example {
using namespace parsers::description;

struct {
  constexpr int operator()(char c) const noexcept { return c - '0'; }
} constexpr to_int;
struct {
  template <class... Ts>
  constexpr int operator()(const std::tuple<Ts...>& tpl) const noexcept {
    static_assert(std::conjunction_v<std::is_convertible<Ts, int>...>);
    return _add(std::move(tpl), std::index_sequence_for<Ts...>{});
  }

 private:
  template <std::size_t... Ss, class T>
  constexpr int _add(T&& tuple, std::index_sequence<Ss...>) const noexcept {
    return (std::get<Ss>(tuple) + ...);
  }
} constexpr add;

constexpr auto _1 = map{ascii::digit, to_int};
constexpr auto _2 = map{sequence{_1, _1, _1, _1}, add};
}  // namespace example

TEST(Map, MatcherShouldWorkUnchanged) {
  using namespace example;

  static_assert(parsers::match(_1, "1"));
  static_assert(parsers::match(both{many{_1}, eos}, "1123"));
  static_assert(!parsers::match(_1, "a"));
  static_assert(parsers::match(both{_2, eos}, "1111"));
}

template <class T, class U>
constexpr bool check(T&& t, U&& u) noexcept {
  return parsers::range{std::get<0>(t), std::get<1>(t)} == std::forward<U>(u);
}

TEST(Map, RangeShouldWorkUnchanged) {
  using namespace example;

  constexpr auto p1 = parsers::parse_range(_1, "1");
  static_assert(p1.has_value());
  static_assert(check(p1.value(), "1"));

  constexpr auto p2 = parsers::parse_range(both{many{_1}, eos}, "1123");
  static_assert(p2.has_value());
  static_assert(check(p2.value(), "1123\0"));

  constexpr auto p3 = parsers::parse_range(_1, "a");
  static_assert(!p3.has_value());

  constexpr auto p4 = parsers::parse_range(both{_2, eos}, "4444");
  static_assert(p4.has_value());
  static_assert(check(p4.value(), "4444\0"));
}

TEST(Map, ObjectMapsValuesCorrectly) {
  using namespace example;

  constexpr auto p1 = parsers::parse(_1, "1");
  static_assert(p1.has_value());
  static_assert(p1.value() == 1);

  auto p2 = parsers::parse(both{many{_1}, discard{eos}}, "1234");
  ASSERT_TRUE(p2.has_value());
  auto r2 = std::move(p2).value();
  ASSERT_EQ(r2.size(), 4);
  for (int i = 0; i < 4; ++i) {
    ASSERT_EQ(r2[i], i + 1);
  }

  constexpr auto p3 = parsers::parse(_2, "4444");
  static_assert(p3.has_value());
  static_assert(p3.value() == 16);

  constexpr auto p4 = parsers::parse(both{_2, discard{eos}}, "4444");
  static_assert(p4.has_value());
  static_assert(p4.value() == 16);
}