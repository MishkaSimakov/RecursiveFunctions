#include "CppCompiler.h"

namespace Compilation {
void CppCompiler::compile(const VariableNode& node) {
  result_ = get_variable_name(node.index);
}
}  // namespace Compilation