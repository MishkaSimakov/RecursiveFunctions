#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const FunctionDefinitionNode& node) {
  wrap_with_function(node.name, node.arguments_count, [&node, this] {
    current_function_->set_begin_block(compile_node(node.body));
  });
}
}  // namespace IR
