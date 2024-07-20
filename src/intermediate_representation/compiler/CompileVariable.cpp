#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const VariableNode& node) {
  assign_or_pass_as_argument(TemporaryOrConstant::temporary(node.index));
}
}  // namespace IR
