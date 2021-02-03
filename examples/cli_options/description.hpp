#ifndef GUARD_EXAMPLES_CLI_OPTIONS_DESCRIPTION_HPP
#define GUARD_EXAMPLES_CLI_OPTIONS_DESCRIPTION_HPP

#include <parsers/parsers.hpp>
#include "parsers/description.hpp"
#include "parsers/description/ascii.hpp"

namespace cli_options {
using namespace parsers::description;
using ascii::alpha;
using ascii::whitespace;

using dash = character<'-'>;
using double_dash = both<dash, dash>;

using short_option_name = both<dash, alpha>;
using long_option_name =
    sequence<double_dash,
             alpha,
             many<alternative<alpha, character<'-'>, character<'_'>>>>;

using quotation_mark = character<'"'>;
using backslash = character<'\\'>;
using escaped_quote = both<backslash, quotation_mark>;

struct non_quote : satisfy<non_quote> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return c != '"';
  }
};

using string = sequence<quotation_mark,
                        many<either<escaped_quote, non_quote>>,
                        quotation_mark>;

struct value_characters : satisfy<value_characters> {
  template <class T>
  constexpr bool operator()(T c) const noexcept {
    return !(ascii::isspace(c) || ascii::isctrl(c) || c == '"');
  }
};
using value = either<string, many<value_characters>>;

using short_option = sequence<short_option_name, many<whitespace>, value>;
using long_option = sequence<
    long_option_name,
    either<sequence<many<whitespace>, character<'='>, many<whitespace>>,
           many1<whitespace>>,
    value>;

using option_list =
    both<many<both<many1<whitespace>, either<short_option, long_option>>>,
         many<whitespace>>;

}  // namespace cli_options

#endif  // GUARD_EXAMPLES_CLI_OPTIONS_DESCRIPTION_HPP