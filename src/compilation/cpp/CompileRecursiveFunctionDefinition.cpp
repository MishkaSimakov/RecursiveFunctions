#include "CppCompiler.h"

namespace Compilation {
void CppCompiler::compile(const RecursiveFunctionDefinitionNode& node) {
  result_ = string("int ") + node.name + "(";

  for (size_t i = 0; i < node.arguments_count; ++i) {
    result_ += get_variable_name(i);

    if (i + 1 != node.arguments_count) {
      result_ += ", ";
    }
  }

  result_ += ") {";
  result_ += "if (" + get_variable_name(0) + " != 0) {";
  result_ += "return " + compile_node(node.general_case);
  result_ += "}";
  result_ += "return " + compile_node(node.zero_case);
}
}  // namespace Compilation