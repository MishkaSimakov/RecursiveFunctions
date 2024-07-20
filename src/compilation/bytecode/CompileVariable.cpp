#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::visit(const VariableNode& node) {
  int offset = -(static_cast<int>(node.index) + 1) * 8;
  string variable_pointer = fmt::format("[x29, {}]", get_constant(offset));
  string return_register = get_register(current_argument_offset);

  // load variable from stack
  result_.emplace_back("ldr", return_register, variable_pointer);
}
}  // namespace Compilation
