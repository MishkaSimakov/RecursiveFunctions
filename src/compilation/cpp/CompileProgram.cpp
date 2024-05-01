#include "CppCompiler.h"

namespace Compilation {
void CppCompiler::compile(const ProgramNode& node) {
  string result;
  for (auto& statement : node.functions) {
    result += compile_node(statement);
  }

  result += "int main() {";
  result += compile_node(node.call);
  result += "}";

  result_ = std::move(result);
}
}  // namespace Compilation