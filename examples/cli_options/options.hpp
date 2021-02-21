#ifdef GUARD_CLI_OPTIONS_OPTION_HPP
#define GUARD_CLI_OPTIONS_OPTION_HPP

#include <utility>

namespace cli_options {
template <class T>
struct type_t {
  using type = T;
};
template <class T>
constexpr static inline type_t<T> type{};

template <class T, std::size_t Size>
struct array {
  static_assert(Size > 0);

  using value_type = T;

  constexpr array(const value_type (&name)[Size]) noexcept
      : array{name, std::make_index_sequence<Size>{}} {}

  value_type value[Size];
  [[nodiscard]] constexpr static inline std::size_t size() noexcept {
    return Size;
  }

 private:
  template <std::size_t... Ss>
  constexpr array(const value_type (&name)[Size],
                  [[maybe_unused]] std::index_sequence<Ss...>) noexcept
      : value{name[Ss]...} {}
};

template <std::size_t Size>
struct long_name_t : private array<char, Size> {
  using base = array<char, Size>;

  constexpr long_name_t(const char (&val)[Size]) noexcept : base{val} {}

  [[nodiscard]] constexpr const char (&long_name() const noexcept)[Size] {
    return base::value;
  }
};

template <std::size_t Size>
struct description_t : private array<char, Size> {
  using base = array<char, Size>;

  constexpr description_t(const char (&val)[Size]) noexcept : base{val} {}

  [[nodiscard]] constexpr const char (&description() const noexcept)[Size] {
    return base::value;
  }
};

struct short_name_t {
  constexpr short_name_t(char c) noexcept : _value{c} {}

  [[nodiscard]] constexpr char short_name() const noexcept { return _value; }

 private:
  char _value;
};

template <class... Ts>
struct option : Ts... {
  template <class... Us>
  constexpr option(Us&&... us) noexcept : Ts{std::forward<Us>(us)}... {}
};
template <std::size_t LN, std::size_t D, class T>
option(char, const char (&)[LN], type_t<T>, const char (&)[D])
    -> option<short_name_t, long_name_t<LN>, type_t<T>, description_t<D>>;

template <std::size_t LN, std::size_t D, class T>
option(const char (&)[LN], char, type_t<T>, const char (&)[D])
    -> option<long_name_t<LN>, short_name_t, type_t<T>, description_t<D>>;

template <std::size_t LN, class T>
option(const char (&)[LN], char, type_t<T>)
    -> option<long_name_t<LN>, short_name_t, type_t<T>>;

template <std::size_t LN, class T>
option(char, const char (&)[LN], type_t<T>)
    -> option<short_name_t, long_name_t<LN>, type_t<T>>;

template <std::size_t LN, class T>
option(const char (&)[LN], type_t<T>) -> option<long_name_t<LN>, type_t<T>>;

template <class T>
option(char, type_t<T>) -> option<short_name_t, type_t<T>>;

template <std::size_t LN, class T, std::size_t D>
option(const char (&)[LN], type_t<T>, const char (&)[D])
    -> option<long_name_t<LN>, type_t<T>, description_t<D>>;

template <class T, std::size_t D>
option(char, type_t<T>, const char (&)[D])
    -> option<short_name_t, type_t<T>, description_t<D>>;

}  // namespace cli_options

#endif  // GUARD_CLI_OPTIONS_OPTION_HPP