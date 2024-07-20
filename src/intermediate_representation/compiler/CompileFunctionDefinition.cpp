#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const FunctionDefinitionNode& node) {
  wrap_with_function(node.name, node.arguments_count, [&node, this] {
    node.body->accept(*this);
  });
}
}  // namespace IR
