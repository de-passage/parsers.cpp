#ifndef GUARD_PARSERS_RANGE_HPP
#define GUARD_PARSERS_RANGE_HPP

#include <iterator>
#include <type_traits>

#include "./utility.hpp"

namespace parsers {

struct zero_marker {};

namespace detail {
template <bool Start, class It>
struct range_iterator_container {
  using iterator_type = It;

  constexpr explicit range_iterator_container() = default;
  template <class I,
            std::enable_if_t<
                !std::is_same_v<range_iterator_container, std::decay_t<I>>,
                int> = 0>
  constexpr explicit range_iterator_container(I&& v) noexcept
      : _value(std::forward<I>(v)) {}

  [[nodiscard]] constexpr iterator_type value() const noexcept {
    return _value;
  }

 private:
  It _value;
};

template <bool B>
struct range_iterator_container<B, zero_marker> {
  using iterator_type = zero_marker;

  [[nodiscard]] static constexpr iterator_type value() noexcept {
    return zero_marker{};
  }
};

template <class C>
struct const_char_wrapper {
  using value_type = typename std::iterator_traits<const C*>::value_type;
  using pointer = typename std::iterator_traits<const C*>::pointer;
  using difference_type =
      typename std::iterator_traits<const C*>::difference_type;
  using reference = typename std::iterator_traits<const C*>::reference;
  using iterator_category = typename std::bidirectional_iterator_tag;

  constexpr const_char_wrapper() noexcept = default;
  template <
      class T,
      std::enable_if_t<!std::is_same_v<std::decay_t<T>, const_char_wrapper>,
                       int> = 0>
  constexpr explicit const_char_wrapper(T&& ptr) noexcept
      : _ptr{static_cast<const C*>(ptr)} {}

 private:
  const C* _ptr;

 public:
  constexpr operator const C*() const noexcept { return _ptr; }
  constexpr const_char_wrapper& operator++() noexcept {
    ++_ptr;
    return *this;
  }
  constexpr const_char_wrapper& operator--() noexcept {
    --_ptr;
    return *this;
  }
  constexpr const_char_wrapper operator++(int) noexcept {
    auto c = *this;
    ++*this;
    return c;
  }
  constexpr const_char_wrapper operator--(int) noexcept {
    auto c = *this;
    --*this;
    return c;
  }

  constexpr reference operator*() const noexcept { return *_ptr; }

  friend constexpr bool operator==(const_char_wrapper left,
                                   const_char_wrapper right) noexcept {
    return left._ptr == right._ptr;
  }

  friend constexpr bool operator==(const_char_wrapper left,
                                   const C* right) noexcept {
    return left._ptr == right;
  }

  friend constexpr bool operator==(
      const_char_wrapper left,
      [[maybe_unused]] zero_marker right) noexcept {
    return *left._ptr == 0;
  }

  template <class T>
  friend constexpr bool operator==(T left, const_char_wrapper right) noexcept {
    return right == left;
  }

  template <class T>
  friend constexpr bool operator!=(const_char_wrapper left, T right) noexcept {
    return !(left == right);
  }

  template <
      class T,
      std::enable_if_t<!std::is_same_v<std::decay_t<T>, const_char_wrapper>,
                       int> = 0>
  friend constexpr bool operator!=(T left, const_char_wrapper right) noexcept {
    return !(left == right);
  }

  [[nodiscard]] constexpr auto distance() const noexcept {
    return std::distance(begin(), end());
  }
};

template <bool B, class T>
struct range_iterator_container<B, T*> {
  using iterator_type = const_char_wrapper<T>;

  constexpr explicit range_iterator_container(const T* v) noexcept
      : _value(v) {}

  [[nodiscard]] constexpr iterator_type value() const noexcept {
    return _value;
  }

 private:
  iterator_type _value;
};

static_assert(
    std::is_same_v<
        typename std::iterator_traits<const_char_wrapper<char>>::value_type,
        char>);

}  // namespace detail

template <class ItB, class ItE>
struct range : private detail::range_iterator_container<true, ItB>,
               private detail::range_iterator_container<false, ItE> {
  using first_t = ItB;
  using second_t = ItE;

  using begin_container = detail::range_iterator_container<true, first_t>;
  using end_container = detail::range_iterator_container<false, second_t>;

  using begin_iterator = typename begin_container::iterator_type;
  using end_iterator = typename end_container::iterator_type;

  using value_type = typename std::iterator_traits<begin_iterator>::value_type;

  template <class JtB, class JtE>
  constexpr range(JtB&& begin, JtE&& end) noexcept
      : begin_container(std::forward<JtB>(begin)),
        end_container(std::forward<JtE>(end)) {}

  template <class JtB,
            std::enable_if_t<!std::is_same_v<range, std::decay_t<JtB>> &&
                                 std::is_default_constructible_v<end_container>,
                             int> = 0>
  constexpr explicit range(JtB&& begin) noexcept
      : begin_container(begin), end_container() {}

  template <
      class JtB,
      std::enable_if_t<!std::is_same_v<range, std::decay_t<JtB>> &&
                           !std::is_default_constructible_v<end_container>,
                       int> = 0>
  constexpr explicit range(JtB&& begin) noexcept
      : begin_container(begin), end_container(begin) {}

  template <std::size_t S, class T>
  constexpr explicit range(const T (&arr)[S]) noexcept
      : begin_container{static_cast<const T*>(arr)},
        end_container{static_cast<const T*>(arr) + S - 1} {}

 private:
  template <class T, class U>
  using similar =
      std::conjunction<std::negation<std::is_same<T, U>>,
                       dpsg::is_template_instance<T, range>,
                       std::is_constructible<typename U::begin_iterator,
                                             typename T::begin_container>,
                       std::is_constructible<typename U::end_iterator,
                                             typename T::end_container>>;

 public:
  template <class T,
            std::enable_if_t<similar<std::decay_t<T>, range>::value, int> = 0>
  constexpr explicit range(T&& val) noexcept
      : begin_container{std::forward<T>(val).begin()},
        end_container{std::forward<T>(val).end()} {}

  template <class T, std::enable_if_t<similar<range, T>::value, int> = 0>
  operator T() const noexcept {
    return T{begin(), end()};
  }

  [[nodiscard]] constexpr begin_iterator begin() const noexcept {
    return begin_container::value();
  }
  [[nodiscard]] constexpr end_iterator end() const noexcept {
    return end_container::value();
  }

  template <class F, class S>
  [[nodiscard]] friend constexpr bool operator==(
      const range& left,
      const range<F, S>& right) noexcept {
    auto beg1 = left.begin();
    const auto end1 = left.end();
    auto beg2 = right.begin();
    const auto end2 = right.end();
    while (true) {
      const auto f1 = beg1 == end1;
      const auto f2 = beg2 == end2;
      if (f1 != f2) {
        return false;
      }
      if (f1 && f2) {
        return true;
      }
      if (*beg1++ != *beg2++) {
        return false;
      }
    }
  }

  template <class T>
  [[nodiscard]] friend constexpr bool operator!=(const range& left,
                                                 T&& right) noexcept {
    return !(left == std::forward<T>(right));
  }

  template <
      class T,
      std::enable_if_t<!dpsg::is_template_instance_v<std::decay_t<T>, range> &&
                           std::is_constructible_v<range, T>,
                       int> = 0>
  [[nodiscard]] friend constexpr bool operator==(const range& left,
                                                 T&& right) noexcept {
    return left == parsers::range{std::forward<T>(right)};
  }

  template <class T,
            std::enable_if_t<!dpsg::is_template_instance_v<T, range>, int> = 0>
  [[nodiscard]] friend constexpr bool operator==(const T& left,
                                                 const range& right) noexcept {
    return right == left;
  }

  template <class T,
            std::enable_if_t<!dpsg::is_template_instance_v<T, range>, int> = 0>
  [[nodiscard]] friend constexpr bool operator!=(const T& left,
                                                 const range& right) noexcept {
    return right != left;
  }
};
template <class T,
          std::enable_if_t<!std::is_array_v<std::remove_reference_t<T>> &&
                               std::is_pointer_v<std::decay_t<T>>,
                           int> = 0>
range(T&& elem) -> range<std::remove_reference_t<T>, zero_marker>;
template <class T, class U>
range(T&&, U&&) -> range<std::decay_t<T>, std::decay_t<U>>;
template <std::size_t S, class T>
range(const T (&)[S]) -> range<const T*, const T*>;
template <class T>
range(detail::const_char_wrapper<T>, detail::const_char_wrapper<T>)
    -> range<T*, T*>;

static_assert(std::is_constructible_v<
              parsers::detail::range_iterator_container<true, const char*>,
              parsers::detail::const_char_wrapper<const char>>);
static_assert(std::is_convertible_v<
              parsers::range<parsers::detail::const_char_wrapper<const char>,
                             parsers::detail::const_char_wrapper<const char>>,
              parsers::range<const char*, const char*>>);
}  // namespace parsers

#endif  // GUARD_PARSERS_RANGE_HPP