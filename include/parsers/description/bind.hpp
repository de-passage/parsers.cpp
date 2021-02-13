#ifndef GUARD_PARSERS_DESCRIPTION_BIND_HPP
#define GUARD_PARSERS_DESCRIPTION_BIND_HPP

#include "../interpreters/make_parser.hpp"
#include "../interpreters/object_parser.hpp"
#include "./modifiers.hpp"

namespace parsers::description {
template <class D, class F>
struct bind
    : modifier<D, interpreters::make_parser_t<interpreters::object_parser>> {
  using base =
      modifier<D, interpreters::make_parser_t<interpreters::object_parser>>;
};

}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_BIND_HPP