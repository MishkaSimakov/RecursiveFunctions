#include "InstructionPrinter.h"

#include "AssemblyPrinter.h"
#include "intermediate_representation/BasicBlock.h"

std::string Assembly::InstructionPrinter::apply(
    const IR::BaseInstruction& instruction,
    const InstructionContext& instruction_context) {
  context_ = &instruction_context;
  result_ = "";

  instruction.accept(*this);
  return std::move(result_);
}

std::string Assembly::InstructionPrinter::print_value(IR::Value value) {
  switch (value.type) {
    case IR::ValueType::CONSTANT:
      return fmt::format("#{}", value.value);
    case IR::ValueType::VIRTUAL_REGISTER:
      throw std::runtime_error("Too late for virtual registers");
    case IR::ValueType::BASIC_REGISTER:
      return fmt::format("x{}", value.value);
    case IR::ValueType::CALLEE_SAVED_REGISTER:
      return fmt::format("x{}", value.value + 19);
    case IR::ValueType::STACK_INDEX:
      return fmt::format("[sp, #{}]", value.value * 8);
  }

  throw std::runtime_error("Unknown value type");
}

void Assembly::InstructionPrinter::visit(const IR::FunctionCall& instruction) {
  result_ = "bl " + AssemblyPrinter::mangle_function_name(instruction.name);
}

void Assembly::InstructionPrinter::visit(const IR::Addition& instruction) {
  result_ = format("add {}, {}, {}", instruction.return_value,
                   instruction.arguments[0], instruction.arguments[1]);
}

void Assembly::InstructionPrinter::visit(const IR::Subtraction& instruction) {
  result_ = format("sub {}, {}, {}", instruction.return_value,
                   instruction.arguments[0], instruction.arguments[1]);
}
void Assembly::InstructionPrinter::visit(const IR::Move& instruction) {
  result_ =
      format("mov {}, {}", instruction.return_value, instruction.arguments[0]);
}
void Assembly::InstructionPrinter::visit(const IR::Phi&) {
  throw std::runtime_error(
      "Phi nodes must be eliminated before generating assembly");
}

void Assembly::InstructionPrinter::visit(const IR::Return& instruction) {}

void Assembly::InstructionPrinter::visit(const IR::Branch& instruction) {
  const auto* current_block = context_->ordering[context_->block_index];
  size_t left_index = context_->get_block_index(current_block->children[0]);
  size_t right_index = context_->get_block_index(current_block->children[1]);

  bool is_left_next = left_index == context_->block_index + 1;
  bool is_right_next = right_index == context_->block_index + 1;

  if (is_left_next) {
    result_ = fmt::format("cbnz {}, {}", print_value(instruction.arguments[0]),
                          context_->labels.at(current_block->children[1]));
  } else if (is_right_next) {
    result_ = fmt::format("cbz {}, {}", print_value(instruction.arguments[0]),
                          context_->labels.at(current_block->children[0]));
  } else {
    result_ = fmt::format("cbnz {}, {}", print_value(instruction.arguments[0]),
                          context_->labels.at(current_block->children[1]));
    result_ +=
        fmt::format("\nb {}", context_->labels.at(current_block->children[0]));
  }
}

void Assembly::InstructionPrinter::visit(const IR::Load& instruction) {
  result_ =
      format("ldr {}, {}", instruction.return_value, instruction.arguments[0]);
}

void Assembly::InstructionPrinter::visit(const IR::Store& instruction) {
  result_ =
      format("str {}, {}", instruction.arguments[0], instruction.arguments[1]);
}
