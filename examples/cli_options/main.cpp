#include <iomanip>
#include <iostream>
#include <type_traits>
#include <utility>

#include "./description.hpp"

namespace parsers {

struct zero_marker {};
namespace detail {
template <bool, class It>
struct range_iterator_container {
  using iterator_type = It;
  It value;
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

static_assert(
    std::is_same_v<
        typename std::iterator_traits<const_char_wrapper<char>>::value_type,
        char>);

template <typename C>
struct range_iterator_container<true, const C*> {
  using iterator_type = const_char_wrapper<C>;
  const_char_wrapper<C> value;
  constexpr explicit range_iterator_container(const C* ptr) : value{ptr} {}
};

template <typename C>
struct range_iterator_container<false, const C*> {
  using iterator_type = zero_marker;
  zero_marker value{};
};

}  // namespace detail

template <class ItB, class ItE>
struct range : private detail::range_iterator_container<true, ItB>,
               private detail::range_iterator_container<false, ItE> {
 private:
  using begin_container = detail::range_iterator_container<true, ItB>;
  using end_container = detail::range_iterator_container<false, ItE>;

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

  constexpr begin_iterator begin() const noexcept {
    return begin_container::value;
  }
  constexpr end_iterator end() const noexcept { return end_container::value; }
};
template <class T>
range(const T* elem) -> range<const T*, const T*>;

}  // namespace parsers

namespace cli_options {
using namespace parsers::description;
using namespace parsers::dsl;

struct word : satisfy_character<word> {
  constexpr auto operator()(
      [[maybe_unused]] const char* element) const noexcept {
    return true;
  }
} constexpr word;

struct match_option : satisfy_character<match_option> {
  template <class C>
  constexpr bool operator()(const C* r) const noexcept {
    return parsers::match(any_option{}, parsers::range{r});
  }
};

struct parse_option : modifier<match_option,
                               parsers::interpreters::make_parser_t<
                                   parsers::interpreters::object_parser>> {
  template <class I>
  using result_t =
      parsers::interpreter_value_type<parsers::interpreters::object_parser,
                                      I,
                                      any_option>;

  template <class C>
  result_t<const C*> operator()(const C* elem) const noexcept {
    auto range = parsers::range{elem};
    return parsers::parse(any_option{}, range).value();
  }

} constexpr test_options;

constexpr auto test =
    many{test_options | construct{parsers::type<std::string>, word}};

}  // namespace cli_options

int main(int argc, const char** argv) {
  using namespace parsers::interpreters;
  const auto parse = make_parser<object_parser>(cli_options::test);
  auto p = parse(argv, argv + argc);
  std::cout << "parsing succeded? " << std::boolalpha << p.has_value()
            << std::endl;
  auto r = std::get<1>(std::move(p).value());
  std::cout << "result size: " << r.size() << std::endl;
  for (std::size_t i = 0; i < r.size(); ++i) {
    std::cout << "n" << i << " ";
    if (r[i].index() == 1) {
      std::cout << "value : " << std::get<1>(r[i]) << std::endl;
    }
    else {
      auto& o = std::get<0>(r[i]);
      std::cout << "option name: " << o.name;
      if (o.value) {
        std::cout << "\tvalue: " << *o.value << std::endl;
      }
      else {
        std::cout << "\t no value" << std::endl;
      }
    }
  }
  return 0;
}