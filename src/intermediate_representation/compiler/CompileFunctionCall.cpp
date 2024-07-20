#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const FunctionCallNode& node) {
  FunctionCall instruction;

  instruction.result_destination = get_next_temporary();
  instruction.name = node.name;
  instruction.arguments.resize(node.arguments.size());

  compiled_calls_stack_.push(std::move(instruction));

  for (current_argument_index_ = 0;
       current_argument_index_ < node.arguments.size();
       ++current_argument_index_) {
    node.arguments[current_argument_index_]->accept(*this);
  }

  result_.instructions.push_back(
      std::make_unique<FunctionCall>(compiled_calls_stack_.top()));
  compiled_calls_stack_.pop();
}
}  // namespace IR
