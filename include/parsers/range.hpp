#ifndef GUARD_PARSERS_RANGE_HPP
#define GUARD_PARSERS_RANGE_HPP

#include <iterator>
#include <type_traits>

namespace parsers {

struct zero_marker {};
namespace detail {
template <class It>
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

template <>
struct range_iterator_container<zero_marker> {
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
  constexpr explicit const_char_wrapper(const C* ptr) noexcept : _ptr{ptr} {}

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
  constexpr const_char_wrapper operator++(int) const noexcept {
    return ++const_char_wrapper{*this};
  }
  constexpr const_char_wrapper operator--(int) const noexcept {
    return --const_char_wrapper{*this};
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
};

template <class T>
struct range_iterator_container<T*> {
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
struct range : private detail::range_iterator_container<ItB>,
               private detail::range_iterator_container<ItE> {
 private:
  using begin_container = detail::range_iterator_container<ItB>;
  using end_container = detail::range_iterator_container<ItE>;

 public:
  using begin_iterator = typename begin_container::iterator_type;
  using end_iterator = typename end_container::iterator_type;

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
        end_container{static_cast<const T*>(arr) + S} {}

  [[nodiscard]] constexpr begin_iterator begin() const noexcept {
    return begin_container::value();
  }
  [[nodiscard]] constexpr end_iterator end() const noexcept {
    return end_container::value();
  }
};
template <class T>
range(const T* elem) -> range<const T*, zero_marker>;
template <class T, class U>
range(T&&, U&&) -> range<std::decay_t<T>, std::decay_t<U>>;
template <std::size_t S, class T>
range(const T (&)[S]) -> range<const T*, const T*>;

}  // namespace parsers

#endif  // GUARD_PARSERS_RANGE_HPP