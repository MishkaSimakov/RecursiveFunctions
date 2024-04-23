#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const RecursionParameterNode& node) {
  result_ = {{InstructionType::COPY, get_recursion_parameter_position()}};
}
}  // namespace Compilation