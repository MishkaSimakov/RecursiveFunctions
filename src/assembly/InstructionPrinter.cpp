#include "InstructionPrinter.h"

#include "AssemblyPrinter.h"
#include "intermediate_representation/BasicBlock.h"

std::list<std::string> Assembly::InstructionPrinter::apply(
    const IR::BaseInstruction& instruction,
    const InstructionContext& instruction_context) {
  context_ = &instruction_context;
  result_.clear();

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
  result_.push_back("bl " +
                    AssemblyPrinter::mangle_function_name(instruction.name));
}

void Assembly::InstructionPrinter::visit(const IR::Addition& instruction) {
  result_.push_back(format("add {}, {}, {}", instruction.return_value,
                           instruction.arguments[0], instruction.arguments[1]));
}

void Assembly::InstructionPrinter::visit(const IR::Subtraction& instruction) {
  result_.push_back(format("sub {}, {}, {}", instruction.return_value,
                           instruction.arguments[0], instruction.arguments[1]));
}
void Assembly::InstructionPrinter::visit(const IR::Move& instruction) {
  result_.push_back(
      format("mov {}, {}", instruction.return_value, instruction.arguments[0]));
}
void Assembly::InstructionPrinter::visit(const IR::Phi&) {
  throw std::runtime_error(
      "Phi nodes must be eliminated before generating assembly");
}

void Assembly::InstructionPrinter::visit(const IR::Return& instruction) {}

void Assembly::InstructionPrinter::visit(const IR::Branch& instruction) {
  const auto* current_block = context_->ordering[context_->block_index];

  bool is_left_next = context_->is_next(current_block->children[0]);
  bool is_right_next = context_->is_next(current_block->children[1]);

  if (is_left_next) {
    result_.push_back(
        fmt::format("cbnz {}, {}", print_value(instruction.arguments[0]),
                    context_->labels.at(current_block->children[1])));
  } else if (is_right_next) {
    result_.push_back(
        fmt::format("cbz {}, {}", print_value(instruction.arguments[0]),
                    context_->labels.at(current_block->children[0])));
  } else {
    result_.push_back(
        fmt::format("cbnz {}, {}", print_value(instruction.arguments[0]),
                    context_->labels.at(current_block->children[1])));
    result_.push_back(
        fmt::format("b {}", context_->labels.at(current_block->children[0])));
  }
}

void Assembly::InstructionPrinter::visit(const IR::Jump&) {}

void Assembly::InstructionPrinter::visit(const IR::Load& instruction) {
  result_.push_back(
      format("ldr {}, {}", instruction.return_value, instruction.arguments[0]));
}

void Assembly::InstructionPrinter::visit(const IR::Store& instruction) {
  result_.push_back(
      format("str {}, {}", instruction.arguments[0], instruction.arguments[1]));
}

void Assembly::InstructionPrinter::visit(const IR::Select& instruction) {
  // csel may be replaced with cset
  auto zero_value = IR::Value(0, IR::ValueType::CONSTANT);
  auto one_value = IR::Value(1, IR::ValueType::CONSTANT);

  result_.push_back(format("cmp {}, {}", instruction.arguments[0], zero_value));

  if (instruction.arguments[1] == zero_value &&
      instruction.arguments[2] == one_value) {
    result_.push_back(format("cset {}, ne", instruction.return_value));
    return;
  }

  if (instruction.arguments[1] == one_value &&
      instruction.arguments[2] == zero_value) {
    result_.push_back(format("cset {}, eq", instruction.return_value));
    return;
  }

  auto copy = instruction;
  std::array<std::string, 2> arguments;

  // if one of arguments is zero constant we replace it with wzr
  size_t current_temp_reg = 14;

  for (size_t i = 1; i <= 2; ++i) {
    if (instruction.arguments[i] == zero_value) {
      arguments[i - 1] = "xzr";
    } else if (instruction.arguments[i].type == IR::ValueType::CONSTANT) {
      auto temp = IR::Value(current_temp_reg++, IR::ValueType::BASIC_REGISTER);
      arguments[i - 1] = print_value(temp);
      result_.push_back(format("mov {}, {}", temp, instruction.arguments[i]));
    } else {
      arguments[i - 1] = print_value(instruction.arguments[i]);
    }
  }

  result_.push_back(fmt::format("csel {}, {}, {}, eq",
                                print_value(instruction.return_value),
                                arguments[0], arguments[1]));
}
