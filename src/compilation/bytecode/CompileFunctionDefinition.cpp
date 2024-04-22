#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const FunctionDefinitionNode& node) {
  compiled_functions_.push_back(compile_node(node.body));
}
}  // namespace Compilation