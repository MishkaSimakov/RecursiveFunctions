#include "InstructionPrinter.h"

std::string Assembly::InstructionPrinter::apply(
    const IR::BaseInstruction& instruction) {
  result_ = "";
  instruction.accept(*this);
  return std::move(result_);
}

void Assembly::InstructionPrinter::visit(const IR::FunctionCall& instruction) {}

void Assembly::InstructionPrinter::visit(const IR::Addition& instruction) {
  result_ = fmt::format("add {}, {}, {}", instruction.return_value,
                        instruction.arguments[0], instruction.arguments[1]);
}

void Assembly::InstructionPrinter::visit(const IR::Subtraction& instruction) {
  result_ = fmt::format("sub {}, {}, {}", instruction.return_value,
                        instruction.arguments[0], instruction.arguments[1]);
}
void Assembly::InstructionPrinter::visit(const IR::Move& instruction) {
  result_ = fmt::format("mov {}, {}", instruction.return_value,
                        instruction.arguments[0]);
}
void Assembly::InstructionPrinter::visit(const IR::Phi&) {
  throw std::runtime_error(
      "Phi nodes must be eliminated before generating assembly");
}

void Assembly::InstructionPrinter::visit(const IR::Return& instruction) {
  result_ = fmt::format("mov x0, {}", instruction.return_value);
}
