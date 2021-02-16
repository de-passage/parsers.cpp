#ifndef GUARD_EXAMPLES_CLI_OPTIONS_DESCRIPTION_HPP
#define GUARD_EXAMPLES_CLI_OPTIONS_DESCRIPTION_HPP

#include <optional>
#include <parsers/parsers.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace cli_options {
using namespace parsers::description;
using ascii::alpha_t;
using ascii::space_t;

template <class T>
using d = discard<T>;
using dash = character<'-'>;
using double_dash = both<dash, dash>;

using short_option_name = both<d<dash>, as_string_view<alpha_t>>;
using long_option_name =
    both<d<double_dash>,
         as_string_view<
             both<alpha_t,
                  many<alternative<alpha_t, character<'-'>, character<'_'>>>>>>;

using quotation_mark = character<'"'>;
using backslash = character<'\\'>;
using escaped_quote = both<backslash, quotation_mark>;

struct non_quote : satisfy_character<non_quote> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c != '"';
  }
};

using string = sequence<d<quotation_mark>,
                        as_string_view<many<either<escaped_quote, non_quote>>>,
                        d<quotation_mark>>;

struct value_characters : satisfy_character<value_characters> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return !(characters::ascii::is_space(c) || characters::ascii::is_cntrl(c) ||
             c == '"');
  }
} constexpr static inline is_value_character;

struct initial_value_character : satisfy_character<initial_value_character> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c != '-' && is_value_character(c);
  }
};
using value = choose<
    string,
    as_string_view<both<initial_value_character, many<value_characters>>>>;

using spaces = d<many<space_t>>;
using spaces1 = d<many1<space_t>>;

template <class T>
struct option {
  std::basic_string_view<T> name;
  std::optional<std::basic_string_view<T>> value;
};
struct to_option {
  template <
      class T,
      class R = typename std::tuple_element_t<0, std::decay_t<T>>::value_type>
  constexpr option<R> operator()(T&& values) const noexcept {
    return option<R>{std::get<0>(std::forward<T>(values)),
                     std::get<1>(std::forward<T>(values))};
  }
};

using short_option =
    map<sequence<short_option_name, optional<both<spaces, value>>>, to_option>;
using long_option = map<
    sequence<long_option_name,
             optional<sequence<
                 d<either<sequence<spaces, character<'='>, spaces>, spaces1>>,
                 value>>>,
    to_option>;

using any_option = choose<short_option, long_option>;

using option_list = both<many<both<spaces1, any_option>>, spaces>;

}  // namespace cli_options

#endif  // GUARD_EXAMPLES_CLI_OPTIONS_DESCRIPTION_HPP