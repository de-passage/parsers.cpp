#include "./description.hpp"
#include "parsers/parsers.hpp"

#include <string_view>

constexpr auto option_list = [] {
  using namespace parsers::dsl;
  return cli_options::option_list{} & eos;
}();

constexpr auto valid = [](const auto& str) {
  return parsers::match_full(option_list, str);
};

static_assert(valid(""));
static_assert(valid("  "));
static_assert(valid(" -p "));
static_assert(valid(" --option "));
static_assert(valid(" -t true "));
static_assert(valid(" -ttrue "));
static_assert(valid(" --option = \"see me\" "));
static_assert(!valid(" --option = \" unterminated "));
static_assert(!valid(" -p\" "));
static_assert(valid(" -p -a"));
static_assert(valid(" -ptest --option \"something \" -a --again = true"));
static_assert(valid(" -p\"\\\"\" "));

template <class D, class S, class E>
constexpr bool check(D&& desc, S&& str, E&& exp) noexcept {
  auto r = parsers::parse(std::forward<D>(desc), std::forward<S>(str));
  if (!r.has_value()) {
    return false;
  }
  return r.value() == exp;
}
static_assert(check(cli_options::string{}, "\"some string\"", "some string"));
static_assert(check(cli_options::long_option_name{}, "--whatever", "whatever"));

constexpr auto r = parsers::parse(cli_options::short_option{}, "-ptest");
static_assert(r.has_value());
static_assert(r.value().name == "p");
static_assert(r.value().value.has_value());
static_assert(r.value().value.value() == "test");

constexpr auto r2 = parsers::parse(cli_options::short_option{}, "-p -test");
static_assert(r2.has_value());
static_assert(r2.value().name == "p");
static_assert(!r2.value().value.has_value());

constexpr auto r3 =
    parsers::parse(cli_options::long_option{}, "--test \"some value\\\"\"");
static_assert(r3.has_value());
static_assert(r3.value().name == "test");
static_assert(r3.value().value.has_value());
static_assert(r3.value().value.value() == "some value\\\"");

constexpr auto r4 =
    parsers::parse(cli_options::long_option{}, "--enable-something");
static_assert(r4.has_value());
static_assert(r4.value().name == "enable-something");
static_assert(!r4.value().value.has_value());

constexpr auto r5 = parsers::parse(cli_options::any_option{}, "-ptest");
static_assert(r5.has_value());
static_assert(r5.value().name == "p");
static_assert(r5.value().value.has_value());
static_assert(r5.value().value.value() == "test");

constexpr auto r6 = parsers::parse(cli_options::any_option{}, "-p -test");
static_assert(r6.has_value());
static_assert(r6.value().name == "p");
static_assert(!r6.value().value.has_value());

constexpr auto r7 =
    parsers::parse(cli_options::any_option{}, "--test \"some value\\\"\"");
static_assert(r7.has_value());
static_assert(r7.value().name == "test");
static_assert(r7.value().value.has_value());
static_assert(r7.value().value.value() == "some value\\\"");

constexpr auto r8 =
    parsers::parse(cli_options::any_option{}, "--enable-something");
static_assert(r8.has_value());
static_assert(r8.value().name == "enable-something");
static_assert(!r8.value().value.has_value());

static_assert(
    std::is_same_v<std::decay_t<decltype(
                       parsers::parse(cli_options::option_list{}, "").value())>,
                   std::vector<cli_options::option<char>>>);