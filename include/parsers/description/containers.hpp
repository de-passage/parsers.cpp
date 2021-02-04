#ifndef GUARD_PARSERS_DESCRIPTION_CONTAINERS_HPP
#define GUARD_PARSERS_DESCRIPTION_CONTAINERS_HPP

#include "../utility.hpp"

namespace parsers::description {

namespace detail {
template <class... Ts>
struct dynamic;

template <class... Ts>
struct tuple;

using parsers::detail::remove_cvref_t;
}  // namespace detail

namespace detail {
template <class It, class Out>
constexpr void copy(It beg, It end, Out out) noexcept {
  while (beg != end) {
    *out++ = *beg++;
  }
}
}  // namespace detail

template <class T>
struct container {
  using parser_t = T;

  constexpr container() noexcept = default;
  template <class U,
            std::enable_if_t<!std::is_array_v<std::remove_reference_t<U>> &&
                                 !std::is_same_v<std::decay_t<U>, container> &&
                                 std::is_convertible_v<U, parser_t>,
                             int> = 0>
  constexpr explicit container(U&& t) noexcept : _parser{std::forward<U>(t)} {}

  template <class U,
            std::enable_if_t<std::is_array_v<std::remove_reference_t<U>> &&
                                 !std::is_same_v<std::decay_t<U>, container>,
                             int> = 0>
  constexpr explicit container(U&& t) noexcept
  // clang-format off
#if defined(_MSC_VER)
      : _parser{}
#endif
  { 
    detail::copy(std::begin(t), std::end(t), std::begin(_parser)); 
  }
  // clang-format on

  constexpr container(container&&) noexcept = default;
  constexpr container(const container& c) noexcept = default;

  [[nodiscard]] constexpr parser_t& parser() & noexcept { return _parser; }
  [[nodiscard]] constexpr const parser_t& parser() const& noexcept {
    return _parser;
  }
  [[nodiscard]] constexpr parser_t&& parser() && noexcept {
    return static_cast<container&&>(*this)._parser;
  }
  [[nodiscard]] constexpr const parser_t&& parser() const&& noexcept {
    return static_cast<const container&&>(*this)._parser;
  }

 private:
  parser_t _parser;
};

template <class T>
struct empty_container {
  using parser_t = T;
  [[nodiscard]] constexpr static inline parser_t parser() noexcept {
    return parser_t{};
  }
};

namespace detail {
template <class T>
struct requires_initialization : std::true_type {};
template <class T>
struct requires_initialization<empty_container<T>> : std::false_type {};
template <class T>
constexpr static inline bool requires_initialization_v =
    requires_initialization<T>::value;

template <std::size_t S, class T, class... Args>
struct at {
  using type = typename at<S - 1, Args...>::type;
};
template <class T, class... Args>
struct at<0, T, Args...> {
  using type = T;
};
template <std::size_t S, class... Args>
using at_t = typename at<S, Args...>::type;

template <std::size_t S, class T>
struct indexed_container : container<T> {
  using base = container<T>;

  constexpr indexed_container() noexcept = default;

  template <
      class U,
      std::enable_if_t<
          std::conjunction_v<
              std::is_constructible<base, U>,
              std::negation<std::is_same<std::decay_t<U>, indexed_container>>>,
          int> = 0>
  constexpr explicit indexed_container(U&& u) : base{std::forward<U>(u)} {}

  constexpr decltype(auto) parser() const& noexcept { return base::parser(); }

  constexpr decltype(auto) parser() & noexcept { return base::parser(); }

  constexpr decltype(auto) parser() && noexcept {
    return static_cast<indexed_container&&>(*this).base::parser();
  }

  constexpr decltype(auto) parser() const&& noexcept {
    return static_cast<const indexed_container&&>(*this).base::parser();
  }

  using parser_t = typename base::parser_t;
};

template <class T, class... Ss>
struct indexed_sequence;
template <std::size_t... Ss, class... Ts>
struct indexed_sequence<std::index_sequence<Ss...>, Ts...>
    : indexed_container<Ss, Ts>... {
  constexpr explicit indexed_sequence() = default;
  template <class... Us>
  constexpr explicit indexed_sequence(Us&&... us)
      : indexed_container<Ss, Ts>{std::forward<Us>(us)}... {}

  template <std::size_t S>
  constexpr decltype(auto) parser() const& noexcept {
    return indexed_container<S, at_t<S, Ts...>>::parser();
  }

  template <std::size_t S>
  constexpr decltype(auto) parser() & noexcept {
    return indexed_container<S, at_t<S, Ts...>>::parser();
  }

  template <std::size_t S>
  constexpr decltype(auto) parser() const&& noexcept {
    return static_cast<const indexed_sequence&&>(*this)
        .indexed_container<S, at_t<S, Ts...>>::parser();
  }

  template <std::size_t S>
  constexpr decltype(auto) parser() && noexcept {
    return static_cast<indexed_sequence&&>(*this)
        .indexed_container<S, at_t<S, Ts...>>::parser();
  }

  template <std::size_t S>
  using parser_t = typename indexed_container<S, at_t<S, Ts...>>::parser_t;

  constexpr static inline std::size_t sequence_length = sizeof...(Ss);
  using parsers = tuple<Ts...>;
};

template <class... Args>
using make_indexed_sequence =
    indexed_sequence<std::index_sequence_for<Args...>, Args...>;

}  // namespace detail
}  // namespace parsers::description

#endif  // GUARD_PARSERS_DESCRIPTION_CONTAINERS_HPP