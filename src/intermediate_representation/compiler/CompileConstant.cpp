#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ConstantNode& node) {
  if (compiled_calls_stack_.empty()) {
    // we should return variable immediately
    result_.end_value = TemporaryOrConstant::constant(node.value);
    return;
  }

  auto& function_call = compiled_calls_stack_.top();

  function_call.arguments[current_argument_index_] =
      TemporaryOrConstant::constant(node.value);
}
}  // namespace IR
