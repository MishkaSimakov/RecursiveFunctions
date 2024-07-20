#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ProgramNode& node) {
  for (auto& function : node.functions) {
    function->accept(*this);
  }

  // wrap main program call inside main function
  auto compiled = compile_node(node.call);
  program_.add_function(Function::entrypoint, std::move(compiled));
}
}  // namespace IR
