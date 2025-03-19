#pragma once
#include <list>
#include <string>
#include <unordered_map>

#include "intermediate_representation/Instruction.h"

namespace Assembly {
struct InstructionContext;

class InstructionPrinter : IR::InstructionVisitor {
  std::list<std::string> result_;
  const InstructionContext* context_{nullptr};

  static std::string format(auto&& s, auto... args) {
    return fmt::format(fmt::runtime(std::forward<decltype(s)>(s)),
                       print_value(args)...);
  }

  static std::string print_value(IR::Value);

  void visit(const IR::FunctionCall&) override;
  void visit(const IR::Addition&) override;
  void visit(const IR::Subtraction&) override;
  void visit(const IR::Move&) override;
  void visit(const IR::Phi&) override;
  void visit(const IR::Return&) override;
  void visit(const IR::Branch&) override;
  void visit(const IR::Jump&) override;
  void visit(const IR::Load&) override;
  void visit(const IR::Store&) override;
  void visit(const IR::Select&) override;

 public:
  std::list<std::string> apply(const IR::BaseInstruction&,
                               const InstructionContext&);
};
}  // namespace Assembly
