#include "BytecodeCompiler.h"
#include "functions/FastAbsoluteDifference.h"
#include "functions/FastAdd.h"

namespace Compilation {
void BytecodeCompiler::compile(const InternalFunctionDefinitionNode& node) {
  if (node.name == "__add") {
    result_ = fast_add_instructions;
  } else if (node.name == "__abs_diff") {
    result_ = fast_absolute_difference_instructions;
  } else if (node.name == "increment") {
    result_ = {{InstructionType::LOAD, 0}, {InstructionType::INCREMENT, 0}};
  }
}
}  // namespace Compilation