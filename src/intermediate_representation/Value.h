#pragma once

#include <fmt/core.h>

#include <string>

#include "syntax/buffalo/SyntaxNode.h"

namespace IR {
enum class ValueType : char {
  CONSTANT,
  VIRTUAL_REGISTER,

  BASIC_REGISTER,
  CALLEE_SAVED_REGISTER,
  STACK_INDEX,
};

struct Value {
  int value{0};
  ValueType type{ValueType::CONSTANT};

  Value() = default;

  Value(int value, ValueType value_type) : value(value), type(value_type) {}

  bool is_temporary() const { return type == ValueType::VIRTUAL_REGISTER; }

  std::string to_string() const {
    char prefix;

    switch (type) {
      case ValueType::CONSTANT:
        prefix = '#';
        break;
      case ValueType::VIRTUAL_REGISTER:
        prefix = '%';
        break;
      case ValueType::BASIC_REGISTER:
        prefix = 'b';
        break;
      case ValueType::CALLEE_SAVED_REGISTER:
        prefix = 'c';
        break;
      case ValueType::STACK_INDEX:
        prefix = 's';
        break;
      default:
        prefix = '?';
        break;
    }

    return prefix + std::to_string(value);
  }

  bool operator==(const Value&) const = default;
};
}  // namespace IR

// fmt library helpers
template <>
struct fmt::formatter<IR::Value> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(IR::Value value, FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.to_string());
  }
};

// hash helpers
namespace std {
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
struct hash<IR::Value> {
  auto operator()(IR::Value value) const {
    auto pair = std::make_pair(value.value, value.type);

    return std::hash<decltype(pair)>()(pair);
  }
};
}  // namespace std
