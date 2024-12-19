#pragma once

template <typename T>
struct EnumInfo;

#define ENUM(name, ...)                                        \
  struct name {                                                \
    enum class InternalEnum : size_t { __VA_ARGS__ };          \
    InternalEnum value_;                                       \
                                                               \
   public:                                                     \
    using enum InternalEnum;                                   \
    static constexpr auto values = std::array{__VA_ARGS__};    \
    static constexpr size_t count = values.size();             \
                                                               \
    name() = delete;                                           \
    constexpr name(InternalEnum value) : value_(value) {}      \
    constexpr operator InternalEnum() const { return value_; } \
    explicit constexpr operator size_t() const {               \
      return static_cast<size_t>(value_);                      \
    }                                                          \
  };
