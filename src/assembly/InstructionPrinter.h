#pragma once
#include "intermediate_representation/Instruction.h"

namespace Assembly {
class InstructionPrinter : IR::InstructionVisitor {
  std::string result_;

  void visit(const IR::FunctionCall&) override;
  void visit(const IR::Addition&) override;
  void visit(const IR::Subtraction&) override;
  void visit(const IR::Move&) override;
  void visit(const IR::Phi&) override;
  void visit(const IR::Return&) override;
  void visit(const IR::Branch&) override;
  void visit(const IR::Load&) override;
  void visit(const IR::Store&) override;

 public:
  std::string apply(const IR::BaseInstruction&);
};
}  // namespace Assembly
