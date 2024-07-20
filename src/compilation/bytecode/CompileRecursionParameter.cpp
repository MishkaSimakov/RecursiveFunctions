#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::visit(const RecursionParameterNode& node) {
  string argument_register = get_register(current_argument_offset);
  result_.emplace_back("mov", argument_register, "x20");
}
}  // namespace Compilation
