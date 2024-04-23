#include "BytecodeCompiler.h"

namespace Compilation {
size_t BytecodeCompiler::get_argmin_parameter_position() const {
  return current_offset_ - argmin_parameter_position_;
}

void BytecodeCompiler::compile(const ArgminCallNode& node) {
  // TODO
}
}  // namespace Compilation