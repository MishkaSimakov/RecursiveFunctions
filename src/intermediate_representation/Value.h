#pragma once

#include <fmt/core.h>

#include <string>

#include "utils/Hashers.h"

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
  Value(const Value&) = default;

  constexpr Value(int value, ValueType value_type)
      : value(value), type(value_type) {}

  Value& operator=(const Value&) = default;

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
  std::strong_ordering operator<=>(const Value&) const = default;
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

template <>
struct std::hash<IR::Value> {
  auto operator()(IR::Value value) const noexcept {
    return tuple_hasher_fn(value.value, value.type);
  }
};
