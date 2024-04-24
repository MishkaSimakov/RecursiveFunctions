#include "BytecodeCompiler.h"

namespace Compilation {
size_t BytecodeCompiler::get_argmin_parameter_position() const {
  return current_offset_ - argmin_parameter_position_;
}

void BytecodeCompiler::compile(const ArgminCallNode& node) {
  argmin_parameter_position_ = current_offset_ + 1;
  auto wrapped = compile_node(node.wrapped_call, 1);

  result_.emplace_back(InstructionType::LOAD_CONST, 0);

  result_.splice(result_.end(), wrapped);

  result_.emplace_back(InstructionType::POP_JUMP_IF_ZERO, result_.size() + 4);
  result_.emplace_back(InstructionType::INCREMENT);
  result_.emplace_back(InstructionType::LOAD_CONST, 0);
  result_.emplace_back(InstructionType::POP_JUMP_IF_ZERO, 1);
}
}  // namespace Compilation