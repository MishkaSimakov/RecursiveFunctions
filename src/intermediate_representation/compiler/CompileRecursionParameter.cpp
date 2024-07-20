#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const RecursionParameterNode& node) {
  assign_or_pass_as_argument(recursion_parameter_temporary_);
}
}  // namespace IR
