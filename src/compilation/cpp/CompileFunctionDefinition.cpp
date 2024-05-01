#include "CppCompiler.h"

namespace Compilation {
void CppCompiler::compile(const FunctionDefinitionNode& node) {
  result_ = string("int ") + node.name + "(";

  for (size_t i = 0; i < node.arguments_count; ++i) {
    result_ += get_variable_name(i);

    if (i + 1 != node.arguments_count) {
      result_ += ", ";
    }
  }

  result_ += ") {";
  result_ += "return " + compile_node(node.body);
  result_ += "}";
}
}  // namespace Compilation