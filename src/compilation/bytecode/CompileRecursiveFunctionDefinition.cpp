#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::visit(const RecursiveFunctionDefinitionNode& node) {
  string mangled_name = get_mangled_name(node.name);
  string cycle_begin_label = mangled_name + "_recursion_cycle_begin";
  string cycle_end_label = mangled_name + "_recursion_cycle_end";

  list<AssemblyInstruction> function_body;

  // save callee-saved registers
  function_body.emplace_back("stp", "x19", "x20", "[sp, #-16]!");
  function_body.emplace_back("str", "x21", "[sp, #-16]!");
  function_body.emplace_back("mov", "x20", get_constant(0));
  function_body.emplace_back("mov", "x21",
                             get_register(node.arguments_count - 1));

  // TODO: eliminate repeating x29 register write
  function_body.emplace_back("mov", "x29", "sp");

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

  // first we do zero case
  function_body.splice(function_body.end(), compile_node(node.zero_case));
  function_body.emplace_back("cmp", "x20", "x21");
  function_body.emplace_back("beq", cycle_end_label);

  // some random alignment
  // TODO: test different cycle alignments later
  function_body.emplace_back(".p2align 5");
  function_body.emplace_back(cycle_begin_label + ":");

  function_body.emplace_back("mov", "x19", "x0");
  function_body.splice(function_body.end(), compile_node(node.general_case));

  function_body.emplace_back("add", "x20", "x20", get_constant(1));
  function_body.emplace_back("cmp", "x20", "x21");
  function_body.emplace_back("bne", cycle_begin_label);

  function_body.emplace_back(cycle_end_label + ":");

  function_body.emplace_back("add", "sp", "sp", get_constant(arguments_size));

  // restore callee-saved registers
  function_body.emplace_back("ldr", "x21", "[sp]", "#16");
  function_body.emplace_back("ldp", "x19", "x20", "[sp]", "#16");

  result_ = decorate_function(mangled_name, std::move(function_body));
}
}  // namespace Compilation
