#include "CppCompiler.h"

namespace Compilation {
void CppCompiler::compile(const ConstantNode& node) {
  result_ = std::to_string(node.value.as_value());
}
}  // namespace Compilation