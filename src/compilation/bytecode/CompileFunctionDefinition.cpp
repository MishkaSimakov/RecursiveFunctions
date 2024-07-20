#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::visit(const FunctionDefinitionNode& node) {
  string mangled_name = get_mangled_name(node.name);

  list<AssemblyInstruction> function_body;

  int arguments_size = node.arguments_count;
  arguments_size += arguments_size % 2;
  arguments_size *= 8;

  function_body.emplace_back("sub", "sp", "sp", get_constant(arguments_size));

  int offset = arguments_size;
  for (size_t i = 0; i < node.arguments_count; ++i) {
    string argument_register = get_register(node.arguments_count - i - 1);

    offset -= 8;
    string pointer = fmt::format("[sp, {}]", get_constant(offset));
    function_body.emplace_back("str", argument_register, pointer);
  }

  function_body.splice(function_body.end(), compile_node(node.body));

  function_body.emplace_back("add", "sp", "sp", get_constant(arguments_size));

  result_ = decorate_function(mangled_name, std::move(function_body));
}
}  // namespace Compilation
