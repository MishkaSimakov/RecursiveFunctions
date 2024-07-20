#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ProgramNode& node) {
  for (auto& statement : node.functions) {
    compiled_functions_.push_back(compile_node(statement));
  }

  auto compiled_call = compile_node(node.call);

  // now we should print value that is stored in x0 register
  compiled_call.emplace_back("str", "x0", "[sp, #-16]!");

  compiled_call.emplace_back("adrp", "x0", "format@PAGE");
  compiled_call.emplace_back("add", "x0", "x0", "format@PAGEOFF");

  compiled_call.emplace_back("mov", "x1", "sp");
  compiled_call.emplace_back("bl", "_printf");

  compiled_call.emplace_back("add", "sp", "sp", get_constant(16));

  compiled_call.emplace_back("mov", "x0", get_constant(0));

  compiled_call_ = decorate_function("_main", compiled_call);

  // store format string in data section
  compiled_call_.emplace_back(".data");
  compiled_call_.emplace_back("format: .asciz \"Result: %i\\n\"");
}
}  // namespace IR
