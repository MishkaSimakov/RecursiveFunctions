#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const ConstantNode& node) {
  result_ = {{InstructionType::LOAD_CONST, node.value.as_value()}};
}
}  // namespace Compilation