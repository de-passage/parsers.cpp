#ifndef GUARD_EXAMPLE_FUNCTIONS_HPP
#define GUARD_EXAMPLE_FUNCTIONS_HPP

#include <functional>

namespace math {
struct power {
  template <class T>
  constexpr auto operator()(T a, T b) noexcept {
    T c = 1;
    while (b-- > 0) {
      c *= a;
    }
    return c;
  }
};

enum class operators { plus, minus, mult, div, pow };

class operator_t {
 public:
  template <class F>
  operator_t(operators op, F&& func)
      : _description{op}, _operation{std::forward<F>(func)} {}

  [[nodiscard]] int operator()(int left, int right) const {
    return _operation(left, right);
  }
  [[nodiscard]] operators description() const { return _description; }

 private:
  operators _description;
  std::function<int(int, int)> _operation;
};
template <auto T, class F>
struct value_t {
  static inline operator_t value() { return {T, F{}}; };
};

template <class T>
struct operator_for;
template <>
struct operator_for<std::plus<>> : value_t<operators::plus, std::plus<>> {};
template <>
struct operator_for<std::minus<>> : value_t<operators::minus, std::minus<>> {};
template <>
struct operator_for<std::multiplies<>>
    : value_t<operators::mult, std::multiplies<>> {};
template <>
struct operator_for<std::divides<>> : value_t<operators::div, std::divides<>> {
};
template <>
struct operator_for<power> : value_t<operators::pow, power> {};
}  // namespace math
#endif  // GUARD_EXAMPLE_FUNCTIONS_HPP