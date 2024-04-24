#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const VariableNode& node) {
  result_ = {{InstructionType::LOAD, node.index}};
}
}  // namespace Compilation