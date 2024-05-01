#include "CppCompiler.h"

namespace Compilation {
void CppCompiler::compile(const FunctionCallNode& node) {
  result_ = node.name + "(";

  for (auto argument_itr = node.arguments.rbegin();
       argument_itr != node.arguments.rend(); ++argument_itr) {
    auto instructions = compile_node(*argument_itr);
    result_.append(instructions);

    if (std::next(argument_itr) != node.arguments.rend()) {
      result_.append(", ");
    }
  }

  result_.append(");");
}
}  // namespace Compilation