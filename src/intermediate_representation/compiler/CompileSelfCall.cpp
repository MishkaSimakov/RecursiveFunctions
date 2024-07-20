#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const SelfCallNode& node) {
  auto function_call = std::make_unique<FunctionCall>();
  auto fresh_temporary = get_next_temporary();

  function_call->result_destination = fresh_temporary;
  function_call->name = node.name;
  function_call->arguments.resize(node.arguments_count);

  for (size_t i = 0; i + 1 < node.arguments_count; ++i) {
    function_call->arguments[i] = TemporaryOrConstant::temporary(i);
  }

  function_call->arguments.back() = recursion_parameter_temporary_;

  result_->instructions.push_back(std::move(function_call));

  assign_or_pass_as_argument(fresh_temporary);
}
}  // namespace IR
