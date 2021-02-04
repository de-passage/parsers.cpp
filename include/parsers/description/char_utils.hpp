#ifndef GUARD_PARSERS_CHARACTERS_UTILS_HPP
#define GUARD_PARSERS_CHARACTERS_UTILS_HPP

namespace characters {
namespace ascii {

struct to_upper_t {
  constexpr to_upper_t() noexcept {
    for (unsigned char c = 0; c < 128; ++c) {
      if (c >= 'A' && c <= 'Z') {
        storage[c] = static_cast<char>(c & 32);
      }
      else {
        storage[c] = static_cast<char>(c);
      }
    }
  }
  char storage[128]{};

  constexpr char operator()(char c) const noexcept {
    return storage[static_cast<unsigned char>(c)];
  }
};
constexpr static inline to_upper_t to_upper{};

struct to_lower_t {
  constexpr to_lower_t() noexcept {
    for (unsigned char c = 0; c < 128; ++c) {
      if (c >= 'A' && c <= 'Z') {
        storage[c] = static_cast<char>(c | 32);
      }
      else {
        storage[c] = static_cast<char>(c);
      }
    }
  }
  char storage[128]{};

  constexpr char operator()(char c) const noexcept {
    return storage[static_cast<unsigned char>(c)];
  }
};
constexpr static inline to_lower_t to_lower{};

struct toggle_case_t {
  constexpr toggle_case_t() noexcept {
    for (unsigned char c = 0; c < 128; ++c) {
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        storage[c] = static_cast<char>(c ^ 32);
      }
      else {
        storage[c] = static_cast<char>(c);
      }
    }
  }
  char storage[128]{};

  constexpr char operator()(char c) const noexcept {
    return storage[static_cast<unsigned char>(c)];
  }
};
constexpr static inline toggle_case_t toggle_case{};

}  // namespace ascii

}  // namespace characters

#endif  // GUARD_PARSERS_CHARACTERS_UTILS_HPP