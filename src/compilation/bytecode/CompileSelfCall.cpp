#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const SelfCallNode& node) {
  result_ = {{InstructionType::COPY, get_recursion_call_result_position()}};
}
}  // namespace Compilation