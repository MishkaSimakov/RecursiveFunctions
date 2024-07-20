#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ConstantNode& node) {
  assign_or_pass_as_argument(TemporaryOrConstant::constant(node.value));
}
}  // namespace IR
