#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const VariableNode& node) {
  if (compiled_calls_stack_.empty()) {
    // we should return variable immediately
    result_.end_value = TemporaryOrConstant::temporary(node.index);
    return;
  }

  auto& function_call = compiled_calls_stack_.top();

  function_call.arguments[current_argument_index_] =
      TemporaryOrConstant::temporary(node.index);
}
}  // namespace IR
