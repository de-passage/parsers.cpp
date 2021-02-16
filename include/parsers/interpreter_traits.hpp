#ifndef GUARD_PARSERS_CUSTOMIZATION_POINTS_HPP
#define GUARD_PARSERS_CUSTOMIZATION_POINTS_HPP

#include "./interpreters.hpp"
#include "./result_traits.hpp"

namespace parsers::customization_points {

template <class I>
struct interpreter_traits {
  template <class It, class Desc>
  using result_type = typename I::template result_t<It, Desc>;

  template <class It, class Desc>
  using value_type = typename result_traits<
      typename I::template result_t<It, Desc>>::value_type;
};
}  // namespace parsers::customization_points

namespace parsers {
template <class I>
using interpreter_traits = customization_points::interpreter_traits<I>;

template <class Interpreter, class Iterator, class Description>
using interpreter_value_type =
    typename interpreter_traits<Interpreter>::template value_type<Iterator,
                                                                  Description>;

template <class Interpreter, class Iterator, class Description>
using interpreter_result_type =
    typename interpreter_traits<Interpreter>::template result_type<Iterator,
                                                                   Description>;
}  // namespace parsers

#endif  // GUARD_PARSERS_CUSTOMIZATION_POINTS_HPP