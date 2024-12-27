#pragma once

#include <array>
#include <string_view>

template <typename T>
struct EnumInfo;

#define ENUM(name, ...)                                                      \
  struct name {                                                              \
    enum class InternalEnum : size_t { __VA_ARGS__ };                        \
    InternalEnum value_;                                                     \
                                                                             \
   public:                                                                   \
    using enum InternalEnum;                                                 \
    static constexpr auto values = std::array{__VA_ARGS__};                  \
    static constexpr size_t count = values.size();                           \
                                                                             \
   private:                                                                  \
    static constexpr auto separated = [] {                                   \
      std::array<std::string_view, count> result;                            \
                                                                             \
      size_t current = 0;                                                    \
      size_t current_begin = 0;                                              \
      std::string_view string = #__VA_ARGS__;                                \
      for (size_t i = 0; i < string.size(); ++i) {                           \
        if (string[i] == ',') {                                              \
          result[current] = string.substr(current_begin, i - current_begin); \
          ++current;                                                         \
          current_begin = i + 2;                                             \
          ++i;                                                               \
        }                                                                    \
      }                                                                      \
      result.back() = string.substr(current_begin);                          \
                                                                             \
      return result;                                                         \
    }();                                                                     \
                                                                             \
   public:                                                                   \
    name() = delete;                                                         \
    constexpr name(InternalEnum value) : value_(value) {}                    \
    explicit constexpr name(size_t value)                                    \
        : value_(static_cast<InternalEnum>(value)) {}                        \
    constexpr operator InternalEnum() const { return value_; }               \
    explicit constexpr operator size_t() const {                             \
      return static_cast<size_t>(value_);                                    \
    }                                                                        \
                                                                             \
    bool operator==(InternalEnum other) const { return value_ == other; }    \
    bool operator==(name other) const { return value_ == other.value_; }     \
                                                                             \
    constexpr std::string_view toString() const {                            \
      return separated[static_cast<size_t>(value_)];                         \
    }                                                                        \
  };
