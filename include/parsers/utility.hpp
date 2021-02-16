#ifndef GUARD_PARSERS_UTILITY_HPP
#define GUARD_PARSERS_UTILITY_HPP

#include "./utility/feed.hpp"
#include "./utility/is_template_instance.hpp"
#include "./utility/result.hpp"

#include <type_traits>
#include <utility>

namespace parsers {
struct empty {};
template <class T>
struct type_t {
  using type = T;
};
template <class T>
constexpr static inline type_t<T> type{};

template <class... Ts>
struct type_list_t {};
template <class... Ts>
constexpr static inline type_list_t type_list{};

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

  [[nodiscard]] constexpr const T& get() const& { return *_ptr; };
  [[nodiscard]] constexpr T& get() & { return *_ptr; };
  [[nodiscard]] constexpr const T&& get() const&& {
    return *static_cast<const reference_wrapper&&>(*this)._ptr;
  };
  [[nodiscard]] constexpr T&& get() && {
    return *static_cast<reference_wrapper&&>(*this)._ptr;
  };

  [[nodiscard]] constexpr T* operator->() noexcept { return _ptr; }
  [[nodiscard]] constexpr const T* operator->() const noexcept { return _ptr; }

  [[nodiscard]] constexpr operator T&() & noexcept { return *_ptr; }
  [[nodiscard]] constexpr operator const T&() const& noexcept { return *_ptr; }

  [[nodiscard]] constexpr operator T&&() && noexcept {
    return *static_cast<reference_wrapper&&>(*this)._ptr;
  }
  [[nodiscard]] constexpr operator const T&&() const&& noexcept {
    return *static_cast<const reference_wrapper&&>(*this)._ptr;
  }

 private:
  T* _ptr;
};

template <class T>
[[nodiscard]] constexpr reference_wrapper<T> ref(T& t) noexcept {
  return reference_wrapper<T>{t};
}

template <class A>
[[nodiscard]] constexpr decltype(auto) last_of(A&& a) noexcept {
  return std::forward<A>(a);
}

template <class A, class B, class... Args>
[[nodiscard]] constexpr decltype(auto) last_of([[maybe_unused]] A&& a,
                                               B&& b,
                                               Args&&... args) noexcept {
  return last_of(std::forward<B>(b), std::forward<Args>(args)...);
}

template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
}  // namespace detail

}  // namespace parsers

#endif  // GUARD_PARSERS_UTILITY_HPP