#include "BytecodeCompiler.h"

namespace Compilation {
size_t BytecodeCompiler::get_recursion_call_result_position() const {
  return get_recursion_parameter_position() - 1;
}

size_t BytecodeCompiler::get_recursion_parameter_position() const {
  return current_offset_ - recursion_parameter_position_;
}

void BytecodeCompiler::compile(const RecursiveFunctionDefinitionNode& node) {
  list<Instruction> compiled;

  if (node.use_previous_value) {
    auto zero_case = compile_node(node.zero_case);

    recursion_parameter_position_ = current_offset_ + 3;
    auto general_case = compile_node(node.general_case, 4);

    //
    size_t offset = zero_case.size() + general_case.size();

    compiled.splice(compiled.end(), zero_case);

    /* 1 */ compiled.emplace_back(InstructionType::LOAD, 0);

    // jump to the end
    /* 2 */ compiled.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                                  offset + 16);

    // load variables for recursion cycle
    /* 3 */ compiled.emplace_back(InstructionType::LOAD, 0);
    /* 4 */ compiled.emplace_back(InstructionType::DECREMENT);
    /* 5 */ compiled.emplace_back(InstructionType::LOAD_CONST, 0);
    /* 6 */ compiled.emplace_back(InstructionType::COPY, 2);

    compiled.splice(compiled.end(), general_case);

    // leave cycle if we calculated last value
    /* 7 */ compiled.emplace_back(InstructionType::POP, 1);
    /* 8 */ compiled.emplace_back(InstructionType::COPY, 2);
    /* 9 */ compiled.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                                  offset + 13);

    // increment loop counter
    /* 10 */ compiled.emplace_back(InstructionType::INCREMENT, 1);
    /* 11 */ compiled.emplace_back(InstructionType::DECREMENT, 2);

    // jump back to cycle start
    /* 12 */ compiled.emplace_back(InstructionType::LOAD_CONST, 0);
    /* 13 */ compiled.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                                   zero_case.size() + 7);

    /* 14 */ compiled.emplace_back(InstructionType::POP, 1);
    /* 15 */ compiled.emplace_back(InstructionType::POP, 1);
    /* 16 */ compiled.emplace_back(InstructionType::POP, 1);
  } else {
    auto zero_case = compile_node(node.zero_case, 1);

    recursion_parameter_position_ = current_offset_ + 1;
    auto general_case = compile_node(node.general_case, 1);

    //
    compiled.emplace_back(InstructionType::LOAD, 0);
    compiled.emplace_back(InstructionType::JUMP_IF_NONZERO,
                          zero_case.size() + 4);

    compiled.splice(compiled.end(), zero_case);

    compiled.emplace_back(InstructionType::LOAD_CONST, 0);
    compiled.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                          general_case.size() + zero_case.size() + 6);

    compiled.emplace_back(InstructionType::DECREMENT, 0);
    compiled.splice(compiled.end(), general_case);

    compiled.emplace_back(InstructionType::POP, 1);
  }

  result_ = std::move(compiled);
}
}  // namespace Compilation