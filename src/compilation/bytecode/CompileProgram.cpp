#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const ProgramNode& node) {
  for (auto& statement : node.functions) {
    compiled_functions_.push_back(compile_node(statement));
  }

  compiled_call_ = compile_node(node.call);
}
}  // namespace Compilation