#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const FunctionCallNode& node) {
  auto scratch_temporary = get_next_temporary();

  FunctionCall instruction(scratch_temporary, node.name);

  instruction.arguments.reserve(node.arguments.size());

  compiled_calls_stack_.push(std::move(instruction));

  for (auto& argument : node.arguments) {
    argument->accept(*this);
  }

  result_->instructions.push_back(
      std::make_unique<FunctionCall>(compiled_calls_stack_.top()));

  compiled_calls_stack_.pop();

  assign_or_pass_as_argument(scratch_temporary);
}
}  // namespace IR
