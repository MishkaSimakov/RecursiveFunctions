#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ConstantNode& node) {
  assign_or_pass_as_argument(Value(node.value, ValueType::CONSTANT));
}
}  // namespace IR
