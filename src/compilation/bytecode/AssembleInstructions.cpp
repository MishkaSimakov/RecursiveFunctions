#include "BytecodeCompiler.h"

namespace Compilation {
vector<Instruction> BytecodeCompiler::get_result() const {
  vector<Instruction> result;
  vector<size_t> functions_offsets;
  functions_offsets.reserve(compiled_functions_.size());

  result.insert(result.end(), compiled_call_.begin(), compiled_call_.end());
  result.emplace_back(InstructionType::HALT);

  for (auto& function : compiled_functions_) {
    size_t offset = result.size();
    functions_offsets.push_back(offset);

    for (auto instruction : function) {
      if (instruction.type == InstructionType::POP_JUMP_IF_ZERO ||
          instruction.type == InstructionType::JUMP_IF_NONZERO) {
        instruction.argument += offset;
      }

      result.push_back(instruction);
    }

    result.emplace_back(InstructionType::RETURN);
  }

  for (auto& instruction : result) {
    if (instruction.type == InstructionType::LOAD_CALL) {
      instruction.argument = functions_offsets[instruction.argument];
    }
  }

  return result;
}
}  // namespace Compilation