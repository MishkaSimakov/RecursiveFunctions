#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const VariableNode& node) {
  assign_or_pass_as_argument(Value(node.index, ValueType::VIRTUAL_REGISTER));
}
}  // namespace IR
