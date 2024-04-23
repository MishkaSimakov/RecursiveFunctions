#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const FunctionDefinitionNode& node) {
  result_ = compile_node(node.body);
}
}  // namespace Compilation