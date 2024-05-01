#include "CppCompiler.h"

namespace Compilation {
void CppCompiler::compile(const RecursionParameterNode& node) {
  result_ = get_recursion_parameter_name();
}
}  // namespace Compilation