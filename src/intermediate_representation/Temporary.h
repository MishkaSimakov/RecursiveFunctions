#pragma once

#include <fmt/core.h>

#include <stdexcept>
#include <string>

namespace IR {
struct Temporary {
  size_t index;

  std::string to_string() const { return "%" + std::to_string(index); }

  bool operator==(const Temporary&) const = default;
};

struct TemporaryOrConstant {
  ssize_t index_or_value{0};
  bool is_constant{true};

  TemporaryOrConstant() = default;

  TemporaryOrConstant(ssize_t index_or_value, bool is_constant)
      : index_or_value(index_or_value), is_constant(is_constant) {}

  TemporaryOrConstant(Temporary temporary)
      : index_or_value(temporary.index), is_constant(false) {}

  size_t index() const {
    if (is_constant) {
      throw std::runtime_error("In IR constant used as temporary");
    }

    return index_or_value;
  }

  ssize_t value() const {
    if (!is_constant) {
      throw std::runtime_error("In IR temporary used as constant");
    }

    return index_or_value;
  }

  static TemporaryOrConstant temporary(size_t index) {
    return {static_cast<ssize_t>(index), false};
  }

  static TemporaryOrConstant constant(ssize_t value) { return {value, true}; }

  bool is_temporary() const { return !is_constant; }

  std::string to_string() const {
    if (is_constant) {
      return std::to_string(index_or_value);
    }

    return "%" + std::to_string(index_or_value);
  }

  bool operator==(const TemporaryOrConstant&) const = default;
};
}  // namespace IR

// fmt library helpers
template <>
struct fmt::formatter<IR::TemporaryOrConstant> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const IR::TemporaryOrConstant& value, FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.to_string());
  }
};

template <>
struct fmt::formatter<IR::Temporary> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const IR::Temporary& value, FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.to_string());
  }
};

// hash helpers
namespace std {
template <>
struct hash<IR::Temporary> {
  auto operator()(IR::Temporary temporary) const noexcept {
    return std::hash<size_t>()(temporary.index);
  }
};

template <typename T, typename U>
struct hash<std::pair<T, U>> {
 private:
  // taken from here:
  // https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes
  size_t hash_combine(size_t lhs, size_t rhs) const {
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
  }

 public:
  auto operator()(const std::pair<T, U>& pair) const {
    return hash_combine(std::hash<T>()(pair.first),
                        std::hash<U>()(pair.second));
  }
};

template <>
struct hash<IR::TemporaryOrConstant> {
  auto operator()(IR::TemporaryOrConstant value) const {
    auto pair = std::make_pair(value.index_or_value, value.is_constant);

    return std::hash<decltype(pair)>()(pair);
  }
};
}  // namespace std
