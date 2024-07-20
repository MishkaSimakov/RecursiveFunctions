#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ProgramNode& node) {
  for (auto& function : node.functions) {
    function->accept(*this);
  }

  // wrap main program call inside main function
  wrap_with_function(Function::entrypoint, 0,
                     [&node, this] { node.call->accept(*this); });
}
}  // namespace IR
