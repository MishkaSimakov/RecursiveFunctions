#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const FunctionDefinitionNode& node) {
  current_temporary_index_ = node.arguments_count;

  program_.add_function(node.name, compile_node(node.body));
}
}  // namespace IR
