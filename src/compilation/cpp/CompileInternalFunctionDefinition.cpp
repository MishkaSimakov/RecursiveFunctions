#include "CppCompiler.h"

namespace Compilation {
void CppCompiler::compile(const InternalFunctionDefinitionNode& node) {
  if (node.name == "__add") {
    result_ = get_variable_name(0) + "+" + get_variable_name(1);
  } else if (node.name == "__abs_diff") {
    result_ = "abs(" + get_variable_name(0) + "-" + get_variable_name(1) + ")";
  } else if (node.name == "successor") {
    result_ = get_variable_name(0) + "+1";
  }
}
}  // namespace Compilation