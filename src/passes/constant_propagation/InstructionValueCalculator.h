#pragma once
#include "intermediate_representation/Instruction.h"

namespace Passes {
class InstructionValueCalculator : public IR::InstructionVisitor {
  template <typename T>
  static bool has_const_arguments(const T& instruction) {
    for (auto argument : instruction.arguments) {
      if (argument.type != IR::ValueType::CONSTANT) {
        return false;
      }
    }

    return true;
  }

  std::optional<int> result_;

  void visit(const IR::FunctionCall&) override {}

  void visit(const IR::Addition& instruction) override {
    if (!has_const_arguments(instruction)) {
      return;
    }

    result_ = instruction.arguments[0].value + instruction.arguments[1].value;
  }

  void visit(const IR::Subtraction& instruction) override {
    if (!has_const_arguments(instruction)) {
      return;
    }

    result_ = instruction.arguments[0].value - instruction.arguments[1].value;
  }

  void visit(const IR::Move& instruction) override {
    if (!has_const_arguments(instruction)) {
      return;
    }

    result_ = instruction.arguments[0].value;
  }

  void visit(const IR::Phi&) override {}
  void visit(const IR::Return&) override {}
  void visit(const IR::Branch&) override {}
  void visit(const IR::Jump&) override {}
  void visit(const IR::Load&) override {}
  void visit(const IR::Store&) override {}

  void visit(const IR::Select& instruction) override {
    if (!has_const_arguments(instruction)) {
      return;
    }

    result_ = instruction.arguments[0].value == 0
                  ? instruction.arguments[1].value
                  : instruction.arguments[2].value;
  }

 public:
  IR::Value calculate(const IR::BaseInstruction& instruction) {
    result_ = {};
    instruction.accept(*this);

    if (!result_.has_value()) {
      return ConstantPropagationPass::kBottom;
    }

    return IR::Value(result_.value(), IR::ValueType::CONSTANT);
  }
};
}  // namespace Passes
