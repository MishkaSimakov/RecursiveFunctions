#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const FunctionDefinitionNode& node) {
  current_temporary_index_ = 0;

  node.body->accept(*this);
  program_.functions.emplace_back(
      node.name, std::make_shared<BasicBlock>(std::move(result_)));
}
}  // namespace IR
