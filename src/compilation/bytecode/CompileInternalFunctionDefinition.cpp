#include "BytecodeCompiler.h"
#include "functions/FastAbsoluteDifference.h"
#include "functions/FastAdd.h"

namespace Compilation {
void BytecodeCompiler::visit(const InternalFunctionDefinitionNode& node) {
  list<AssemblyInstruction> body;

  if (node.name == "__add") {
    body = fast_add_instructions;
  } else if (node.name == "__abs_diff") {
    body = fast_absolute_difference_instructions;
  } else if (node.name == "successor") {
    body = {{"add", "x0", "x0", "#1"}};
  }

  result_ =
      decorate_function(get_mangled_name(node.name), std::move(body), true);
}
}  // namespace Compilation
