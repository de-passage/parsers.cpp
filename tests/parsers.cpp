#include <gtest/gtest.h>
#include <optional>
#include <parsers/parsers.hpp>

#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include "parsers/description.hpp"

using namespace parsers::description;

template <class T, class = void>
struct custom_parser;

template <class I>
using parser_result = std::optional<I>;
template <class I>
using int_result = std::optional<std::pair<I, int>>;
template <class T>
constexpr auto make_parser(const T& parser_config) {
  using namespace parsers;

  if constexpr (description::is_failure_v<T>) {
    return []([[maybe_unused]] auto beg,
              [[maybe_unused]] auto end) -> parser_result<decltype(beg)> {
      return {};
    };
  }
  else if constexpr (dpsg::is_template_instance_v<T, either>) {
    return [left = make_parser(parser_config.template parser<0>()),
            right = make_parser(parser_config.template parser<1>())](
               auto beg, auto end) -> parser_result<decltype(beg)> {
      if (auto r = left(beg, end); r.has_value()) {
        return r;
      }
      return right(beg, end);
    };
  }
  else if constexpr (dpsg::is_template_instance_v<T, both>) {
    return [left = make_parser(parser_config.template parser<0>()),
            right = make_parser(parser_config.template parser<1>())](
               auto beg, auto end) -> parser_result<decltype(beg)> {
      if (auto r1 = left(beg, end); r1.has_value()) {
        if (auto r2 = right(*r1, end); r2.has_value()) {
          return r2;
        }
      }
      return {};
    };
  }
  else if constexpr (is_satisfiable_predicate_v<T>) {
    return [parser_config](auto beg, auto end) -> parser_result<decltype(beg)> {
      if (beg == end) {
        return {};
      }
      else if (!parser_config(*beg)) {
        return {};
      }
      return {++beg};
    };
  }
  else if constexpr (dpsg::is_template_instance_v<T, many>) {
    return [parser = make_parser(parser_config.parser())](auto beg, auto end) {
      int count = 0;
      while (beg != end) {
        auto r = parser(beg, end);
        if (!r.has_value()) {
          break;
        }
        beg = *r;
        ++count;
      }
      return int_result<decltype(beg)>{std::in_place, beg, count};
    };
  }
  else {
    return custom_parser<T>::make_parser(parser_config);
  }
}

template <class P, class T>
constexpr bool match(P&& parser, T&& t) noexcept {
  return std::forward<P>(parser)(std::begin(t), std::end(t)).has_value();
}

template <>
struct custom_parser<end_t> {
  constexpr static inline auto make_parser([[maybe_unused]] end_t) noexcept {
    return [](auto beg, auto end) -> parser_result<decltype(beg)> {
      constexpr auto check = [](auto b, auto e) {
        if constexpr (std::is_same_v<std::decay_t<decltype(beg)>,
                                     const char*>) {
          return b == e || *b == 0;
        }
        else {
          return b == e;
        }
      };
      if (check(beg, end)) {
        return {beg};
      }
      return {};
    };
  }
};
template <class T>
struct custom_parser<T, std::enable_if_t<is_recursive_v<T>>> {
  struct parser_t {
    template <class I>
    constexpr inline auto operator()(I beg, I end) const noexcept {
      return ::make_parser(typename T::parser_t{})(beg, end);
    }
  };

  constexpr static inline parser_t make_parser([[maybe_unused]] T _) noexcept {
    return parser_t{};
  }
};

template <auto Char>
struct rec : recursive<either<both<character<Char>, rec<Char>>, end_t>> {};

TEST(Parsers, ShouldCompile) {
  using opt = std::optional<int>;

  static_assert(is_satisfiable_predicate_v<character<'c'>>);
  static_assert(!is_satisfiable_predicate_v<opt>);

  using c = character<'c'>;
  constexpr auto _c = make_parser(c{});
  static_assert(match(_c, "c"));
  static_assert(match(_c, "ca"));
  static_assert(!match(_c, ""));
  static_assert(!match(_c, "!"));

  using d = character<'d'>;
  constexpr auto c_or_d = make_parser(either{c{}, d{}});
  static_assert(match(c_or_d, "c"));
  static_assert(match(c_or_d, "d"));
  static_assert(match(c_or_d, "cd"));
  static_assert(!match(c_or_d, "a"));

  using cd = decltype(both{c{}, d{}});
  constexpr auto _cd = make_parser(cd{});
  static_assert(match(_cd, "cd"));
  static_assert(!match(_cd, "dc"));
  static_assert(!match(_cd, "c"));

  constexpr auto dc = make_parser(both{d{}, c{}});
  static_assert(match(dc, "dca"));
  static_assert(!match(dc, "adc"));

  using any_dc = both<both<any_t, d>, c>;
  constexpr auto _any_dc = make_parser(any_dc{});
  static_assert(match(_any_dc, "adc"));
  static_assert(match(_any_dc, "bdc"));
  static_assert(match(_any_dc, "?dc"));
  static_assert(!match(_any_dc, "?cc"));

  using cd_or_adc = either<cd, any_dc>;
  constexpr auto _cd_or_adc = make_parser(cd_or_adc{});
  static_assert(match(_cd_or_adc, "cd"));
  static_assert(!match(_cd_or_adc, "!cd"));
  static_assert(match(_cd_or_adc, "!dc"));

  ASSERT_TRUE(match(_cd_or_adc, std::string{"!dc is a match"}));

  constexpr auto many_cs = make_parser(many<c>{});
  constexpr auto count = [](auto&& parser, auto&& a) {
    return std::forward<decltype(parser)>(parser)(std::begin(a), std::end(a))
        ->second;
  };
  static_assert(count(many_cs, "cccb") == 3);

  using exactly_c = both<c, end_t>;
  constexpr auto _exactly_c = make_parser(exactly_c{});
  static_assert(match(_exactly_c, "c"));
  static_assert(!match(_exactly_c, "cd"));
  static_assert(!match(_exactly_c, ""));
  static_assert(!match(_exactly_c, "d"));

  using many_c_r = rec<'c'>;
  constexpr auto _many_c_rec = make_parser(many_c_r{});

  static_assert(match(_many_c_rec, "cccc"));
  static_assert(match(_many_c_rec, "c"));
  static_assert(match(_many_c_rec, ""));
  static_assert(!match(_many_c_rec, "d"));
};