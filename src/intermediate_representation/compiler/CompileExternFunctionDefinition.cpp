#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ExternFunctionDefinitionNode& node) {
  program_.extern_functions.insert(node.name);
}
}  // namespace IR
