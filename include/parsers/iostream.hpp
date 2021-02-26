#ifndef GUARD_PARSERS_IOSTREAM_HPP
#define GUARD_PARSERS_IOSTREAM_HPP

#include "./range.hpp"

#include <iostream>

namespace parsers {
template <class F, class S, class... Ts>
std::basic_ostream<Ts...>& operator<<(std::basic_ostream<Ts...>& out,
                                      const range<F, S>& r) {
  auto beg = r.begin();
  const auto end = r.end();
  while (beg != end) {
    out.put(*beg++);
  }
  return out;
}
}  // namespace parsers

#endif  // GUARD_PARSERS_IOSTREAM_HPP