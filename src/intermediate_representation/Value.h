#pragma once

#include <fmt/core.h>

#include <string>

#include "types/Type.h"
#include "utils/Hashers.h"

namespace IR {
class Value {
  Type* type_;

 public:
  explicit Value(Type* type) : type_(type) {}

  virtual std::string to_string() const = 0;

  virtual ~Value() = default;
};

class Temporary : public Value {
  size_t index_;

 public:
  Temporary(Type* type, size_t index) : Value(type), index_(index) {}

  std::string to_string() const override {
    return "%" + std::to_string(index_);
  }
};
}  // namespace IR
