#include <ranges>

#include "BytecodeCompiler.h"

namespace Compilation {

// function always store it's return value in x0
void BytecodeCompiler::visit(const FunctionCallNode& node) {
  list<AssemblyInstruction> compiled;
  size_t arguments_count = node.arguments.size();

  current_argument_offset = 0;

  // we should allocate space for arguments
  int arguments_size = arguments_count;
  arguments_size += arguments_size % 2;
  arguments_size *= 8;

  compiled.emplace_back("sub", "sp", "sp", get_constant(arguments_size));

  int offset = arguments_size;
  for (auto& argument : node.arguments) {
    string argument_register = get_register(current_argument_offset);

    auto instructions = compile_node(argument);
    compiled.splice(compiled.end(), instructions);

    offset -= 8;
    string pointer = fmt::format("[sp, {}]", get_constant(offset));
    compiled.emplace_back("str", argument_register, pointer);

    ++current_argument_offset;
  }

  // now we should load all the arguments from stack
  offset = arguments_size;

  for (size_t i = 0; i < node.arguments.size(); ++i) {
    string register_name = get_register(i);

    offset -= 8;
    string pointer = fmt::format("[sp, {}]", get_constant(offset));

    compiled.emplace_back("ldr", register_name, pointer);
  }

  compiled.emplace_back("add", "sp", "sp", get_constant(arguments_size));

  compiled.emplace_back("bl", get_mangled_name(node.name));
  result_ = std::move(compiled);
}
}  // namespace Compilation
