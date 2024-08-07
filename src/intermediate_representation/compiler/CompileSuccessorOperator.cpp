#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const SuccessorOperatorNode& node) {
  compiled_calls_stack_.emplace(Value{},
                                SuccessorOperatorNode::operator_name);

  node.wrapped->accept(*this);
}
}  // namespace IR
