#ifndef GUARD_PARSERS_CHARACTERS_UTILS_HPP
#define GUARD_PARSERS_CHARACTERS_UTILS_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>

namespace characters {
namespace ascii {

namespace detail {
template <class U, std::size_t N, class G = void>
struct lookup_table {
  constexpr lookup_table() noexcept : lookup_table(G{}) {}
  template <class F>
  constexpr explicit lookup_table(F f) noexcept {
    for (std::size_t i = 0; i < N; ++i) {
      storage[i] = f(i);
    }
  }

  template <class T>
  [[nodiscard]] constexpr U operator()(T in) const noexcept {
    return storage[static_cast<std::make_unsigned_t<T>>(in)];
  }

  U storage[N];
};

using ascii_lookup = lookup_table<char, 128>;

template <auto l, auto L>
struct range {
  template <class T>
  [[nodiscard]] constexpr bool operator()(T v) const noexcept {
    return l <= v && v <= L;
  }
};
template <auto V>
struct value {
  template <class T>
  [[nodiscard]] constexpr bool operator()(T v) const noexcept {
    return v == V;
  }
};

using ctrlcodes1 = range<0, 8>;
using tab = value<9>;
using wspaces = range<10, 13>;
using ctrlcodes2 = range<14, 31>;
using wspace = value<32>;
using special1 = range<33, 47>;
using digit = range<48, 57>;
using special2 = range<58, 64>;
using upperx = range<65, 70>;
using upper_ = range<71, 90>;
using special3 = range<91, 96>;
using lowerx = range<97, 102>;
using lower_ = range<103, 122>;
using special4 = range<123, 126>;
using bspace = value<127>;

template <class... Args>
struct range_checker {
  template <class T>
  [[nodiscard]] constexpr inline bool operator()(T v) const noexcept {
    return (Args{}(v) || ...);
  }
};
template <class... Args>
constexpr static inline range_checker<Args...> check{};

using upper = range_checker<upperx, upper_>;
using lower = range_checker<lowerx, lower_>;
using xdigit = range_checker<digit, lowerx, upperx>;
using alpha = range_checker<lower, upper>;
using alnum = range_checker<alpha, digit>;
using punct = range_checker<special1, special2, special3, special4>;
using graph = range_checker<alnum, punct>;
using print = range_checker<graph, wspace>;
using blank = range_checker<wspace, tab>;
using space = range_checker<blank, wspaces>;
using unprintable_cntrl = range_checker<ctrlcodes1, ctrlcodes2, bspace>;
using cntrl = range_checker<unprintable_cntrl, tab, wspaces>;

template <class R, class F>
struct modify_range {
  constexpr char operator()(std::size_t i) noexcept {
    char c = static_cast<char>(i);
    if (check<R>(c)) {
      return static_cast<char>(F{}(c, 32));
    }
    else {
      return c;
    }
  }
};
using to_upper = modify_range<lower, std::bit_xor<>>;
using to_lower = modify_range<upper, std::bit_or<>>;
using toggle_case = modify_range<alpha, std::bit_xor<>>;

}  // namespace detail

constexpr static inline detail::ascii_lookup to_upper{detail::to_upper{}};
constexpr static inline detail::ascii_lookup to_lower{detail::to_lower{}};
constexpr static inline detail::ascii_lookup toggle_case{detail::toggle_case{}};

namespace detail {
using char_class_t = std::uint16_t;
}

enum class char_class : detail::char_class_t {
  none = 0,
  cntrl = 1,
  print = 1 << 1,
  space = 1 << 2,
  blank = 1 << 3,
  graph = 1 << 4,
  punct = 1 << 5,
  alnum = 1 << 6,
  alpha = 1 << 7,
  upper = 1 << 8,
  lower = 1 << 9,
  digit = 1 << 10,
  xdigit = 1 << 11,
};

namespace detail {

template <template <class> class T>
[[nodiscard]] constexpr inline char_class combine(char_class left,
                                                  char_class right) noexcept {
  return static_cast<char_class>(T<char_class_t>{}(
      static_cast<char_class_t>(left), static_cast<char_class_t>(right)));
}

template <template <class> class T>
[[nodiscard]] constexpr inline char_class& update(char_class& left,
                                                  char_class right) noexcept {
  return left = combine<T>(left, right);
}
}  // namespace detail

[[nodiscard]] constexpr inline char_class operator|(char_class left,
                                                    char_class right) noexcept {
  return detail::combine<std::bit_or>(left, right);
}

[[nodiscard]] constexpr inline char_class operator&(char_class left,
                                                    char_class right) noexcept {
  return detail::combine<std::bit_and>(left, right);
}

[[nodiscard]] constexpr inline char_class operator^(char_class left,
                                                    char_class right) noexcept {
  return detail::combine<std::bit_xor>(left, right);
}

constexpr inline char_class& operator|=(char_class& left,
                                        char_class right) noexcept {
  return detail::update<std::bit_or>(left, right);
}

constexpr inline char_class& operator&=(char_class& left,
                                        char_class right) noexcept {
  return detail::update<std::bit_and>(left, right);
}

constexpr inline char_class& operator^=(char_class& left,
                                        char_class right) noexcept {
  return detail::update<std::bit_xor>(left, right);
}
namespace detail {
template <char_class C, class R>
struct char_class_bit {
  constexpr void operator()(char_class& c, std::size_t s) noexcept {
    if (check<R>(s)) {
      c |= C;
    }
  }
};

template <class... Ts>
struct char_class_builder {
  [[nodiscard]] constexpr char_class operator()(std::size_t s) noexcept {
    auto c = char_class::none;
    (Ts{}(c, s), ...);
    return c;
  }
};

using char_class_lookup =
    char_class_builder<char_class_bit<char_class::cntrl, cntrl>,
                       char_class_bit<char_class::print, print>,
                       char_class_bit<char_class::space, space>,
                       char_class_bit<char_class::blank, blank>,
                       char_class_bit<char_class::graph, graph>,
                       char_class_bit<char_class::punct, punct>,
                       char_class_bit<char_class::alnum, alnum>,
                       char_class_bit<char_class::alpha, alpha>,
                       char_class_bit<char_class::upper, upper>,
                       char_class_bit<char_class::lower, lower>,
                       char_class_bit<char_class::digit, digit>,
                       char_class_bit<char_class::xdigit, xdigit>>;
using char_class_lookup_table =
    lookup_table<char_class, 128, char_class_lookup>;
}  // namespace detail

constexpr static inline detail::char_class_lookup_table lookup_char_class{};

namespace detail {
struct is_class_t {
  template <class T>
  [[nodiscard]] constexpr inline bool operator()(T c,
                                                 char_class cc) const noexcept {
    return (lookup_char_class(c) & cc) != char_class::none;
  }
};

}  // namespace detail
constexpr static inline detail::is_class_t is_class{};

namespace detail {
template <char_class C>
struct test_class_t {
  template <class T>
  [[nodiscard]] constexpr inline bool operator()(T c) const noexcept {
    return is_class_t{}(c, C);
  }
};
}  // namespace detail

using is_cntrl_t = detail::test_class_t<char_class::cntrl>;
using is_print_t = detail::test_class_t<char_class::print>;
using is_space_t = detail::test_class_t<char_class::space>;
using is_blank_t = detail::test_class_t<char_class::blank>;
using is_graph_t = detail::test_class_t<char_class::graph>;
using is_punct_t = detail::test_class_t<char_class::punct>;
using is_alnum_t = detail::test_class_t<char_class::alnum>;
using is_alpha_t = detail::test_class_t<char_class::alpha>;
using is_upper_t = detail::test_class_t<char_class::upper>;
using is_lower_t = detail::test_class_t<char_class::lower>;
using is_digit_t = detail::test_class_t<char_class::digit>;
using is_xdigit_t = detail::test_class_t<char_class::xdigit>;

constexpr static inline is_cntrl_t is_cntrl{};
constexpr static inline is_print_t is_print{};
constexpr static inline is_space_t is_space{};
constexpr static inline is_blank_t is_blank{};
constexpr static inline is_graph_t is_graph{};
constexpr static inline is_punct_t is_punct{};
constexpr static inline is_alnum_t is_alnum{};
constexpr static inline is_alpha_t is_alpha{};
constexpr static inline is_upper_t is_upper{};
constexpr static inline is_lower_t is_lower{};
constexpr static inline is_digit_t is_digit{};
constexpr static inline is_xdigit_t is_xdigit{};

}  // namespace ascii

}  // namespace characters

#endif  // GUARD_PARSERS_CHARACTERS_UTILS_HPP