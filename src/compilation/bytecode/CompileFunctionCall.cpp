#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const FunctionCallNode& node) {
  size_t start_offset = current_offset_;
  ++current_offset_;

  list<Instruction> compiled;
  compiled.emplace_back(InstructionType::LOAD_CALL, node.index);

  for (auto& argument : node.arguments) {
    ++current_offset_;
    auto instructions = compile_node(argument);

    compiled.splice(compiled.end(), instructions);
  }

  compiled.emplace_back(InstructionType::CALL_FUNCTION);

  current_offset_ = start_offset;
  result_ = std::move(compiled);
}
}  // namespace Compilation