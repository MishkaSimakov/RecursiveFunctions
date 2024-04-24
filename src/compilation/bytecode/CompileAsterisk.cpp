#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const AsteriskNode& node) {
  result_ = {{InstructionType::COPY, get_argmin_parameter_position()}};
}
}  // namespace Compilation