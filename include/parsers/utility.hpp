#ifndef GUARD_PARSERS_UTILITY_HPP
#define GUARD_PARSERS_UTILITY_HPP

#include "./is_template_instance.hpp"

#include <utility>

namespace parsers {
struct empty {};
template <class T>
struct type_t {};
template <class T>
constexpr static inline type_t<T> type{};

namespace detail {
template <class T, template <class...> class I>
using instance_of =
    std::enable_if_t<dpsg::is_template_instance_v<std::decay_t<T>, I>, int>;

template <class T, template <class...> class I>
using not_instance_of =
    std::enable_if_t<!dpsg::is_template_instance_v<std::decay_t<T>, I>, int>;

template <class T>
struct reference_wrapper {
  constexpr explicit reference_wrapper(T& ref) : _ptr{&ref} {}

  constexpr const T& get() const& { return *_ptr; };
  constexpr T& get() & { return *_ptr; };
  constexpr const T&& get() const&& {
    return *static_cast<const reference_wrapper&&>(*this)._ptr;
  };
  constexpr T&& get() && {
    return *static_cast<reference_wrapper&&>(*this)._ptr;
  };

  constexpr T* operator->() noexcept { return _ptr; }
  constexpr const T* operator->() const noexcept { return _ptr; }

  constexpr operator T&() & noexcept { return *_ptr; }
  constexpr operator const T&() const& noexcept { return *_ptr; }

  constexpr operator T&&() && noexcept {
    return *static_cast<reference_wrapper&&>(*this)._ptr;
  }
  constexpr operator const T&&() const&& noexcept {
    return *static_cast<const reference_wrapper&&>(*this)._ptr;
  }

 private:
  T* _ptr;
};

template <class T>
constexpr reference_wrapper<T> ref(T& t) noexcept {
  return reference_wrapper<T>{t};
}
}  // namespace detail
}  // namespace parsers

#endif  // GUARD_PARSERS_UTILITY_HPP