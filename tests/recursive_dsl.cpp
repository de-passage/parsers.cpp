#include <parsers/parsers.hpp>

#include <gtest/gtest.h>

using namespace parsers::dsl;

namespace test_detail {
using namespace parsers::description;
template <class R, class T>
using r = ::parsers::dsl::detail::replace_self_t<R, T>;
template <class A, class B>
constexpr static inline bool eq = std::is_same_v<A, B>;

static_assert(eq<r<void, self_t>, void>);
static_assert(eq<r<void, either<both<character<'a'>, self_t>, eos_t>>,
                 either<both<character<'a'>, void>, eos_t>>);

}  // namespace test_detail

TEST(Recursive, CanBeInstanciated) {
  using namespace parsers::description;
  constexpr auto many_a = fix(
      sequence{character{'a'}, either{sequence{character<'a'>{}, self}, eos}});
}
