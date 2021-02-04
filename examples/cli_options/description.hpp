#ifndef GUARD_EXAMPLES_CLI_OPTIONS_DESCRIPTION_HPP
#define GUARD_EXAMPLES_CLI_OPTIONS_DESCRIPTION_HPP

#include <parsers/parsers.hpp>

#include <string>
#include <utility>

namespace cli_options {
using namespace parsers::description;
using ascii::alpha_t;
using ascii::space_t;

using dash = character<'-'>;
using double_dash = both<dash, dash>;
template <class T>
using optional = either<T, succeed_t>;

using short_option_name = both<dash, alpha_t>;
using long_option_name =
    sequence<double_dash,
             alpha_t,
             many<alternative<alpha_t, character<'-'>, character<'_'>>>>;

using quotation_mark = character<'"'>;
using backslash = character<'\\'>;
using escaped_quote = both<backslash, quotation_mark>;

struct non_quote : satisfy_character<non_quote> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c != '"';
  }
};

using string = sequence<quotation_mark,
                        many<either<escaped_quote, non_quote>>,
                        quotation_mark>;

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
using value =
    either<string, both<initial_value_character, many<value_characters>>>;

using short_option =
    sequence<short_option_name, optional<both<many<space_t>, value>>>;
using long_option =
    sequence<long_option_name,
             optional<sequence<
                 either<sequence<many<space_t>, character<'='>, many<space_t>>,
                        many1<space_t>>,
                 value>>>;

using option_list =
    both<many<both<many1<space_t>, either<short_option, long_option>>>,
         many<space_t>>;

}  // namespace cli_options

#endif  // GUARD_EXAMPLES_CLI_OPTIONS_DESCRIPTION_HPP