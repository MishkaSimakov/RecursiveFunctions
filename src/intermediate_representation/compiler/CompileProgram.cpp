#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ProgramNode& node) {
  for (auto& function : node.functions) {
    function->accept(*this);
  }
}
}  // namespace IR
