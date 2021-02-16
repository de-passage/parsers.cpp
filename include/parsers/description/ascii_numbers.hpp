#ifndef GUARD_PARSERS_DESCRIPTION_ASCII_NUMBERS_HPP
#define GUARD_PARSERS_DESCRIPTION_ASCII_NUMBERS_HPP

#include "./ascii.hpp"
#include "./dynamic_range.hpp"

#include "../interpreters/make_parser.hpp"
#include "../interpreters/range_parser.hpp"
#include "./modifiers.hpp"

namespace parsers::description::ascii {

struct number_t : many1<digit_t> {
} constexpr static inline number;

struct integer
    : modifier<number_t,
               interpreters::make_parser_t<interpreters::range_parser>> {
  using base =
      modifier<number_t,
               interpreters::make_parser_t<interpreters::range_parser>>;

  template <class It>
  using result_t = unsigned long long;

  template <class T, class U>
  constexpr int operator()(T beg, const U& end) const noexcept {
    int acc = 0;
    while (beg != end) {
      acc = acc * 10 + (*beg - '0');
      ++beg;
    }
    return acc;
  }
};

}  // namespace parsers::description::ascii

#endif  // GUARD_PARSERS_DESCRIPTION_ASCII_NUMBERS_HPP