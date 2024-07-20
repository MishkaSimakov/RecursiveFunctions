#pragma once

#include <string>
#include <vector>

namespace IR {
struct Temporary {
  size_t index;
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
};

struct Instruction {
  Temporary result_destination;

  virtual ~Instruction() = default;
};

struct FunctionCall final : Instruction {
  std::string name;
  std::vector<TemporaryOrConstant> arguments;
};

struct Addition final : Instruction {
  Temporary left;
  TemporaryOrConstant right;
};

struct Subtraction final : Instruction {
  Temporary left;
  TemporaryOrConstant right;
};

struct Phi final : Instruction {
  std::vector<Temporary> temporaries;
};
}  // namespace IR
