#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ProgramNode& node) {
  for (auto& function : node.functions) {
    function->accept(*this);
  }

  // wrap main program call inside main function
  wrap_with_function(Function::entrypoint, 0, [&node, this] {
    current_function_->set_begin_block(compile_node(node.call));
  });
}
}  // namespace IR
