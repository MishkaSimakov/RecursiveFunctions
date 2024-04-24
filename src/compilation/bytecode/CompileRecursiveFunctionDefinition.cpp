#include "BytecodeCompiler.h"

namespace Compilation {
size_t BytecodeCompiler::get_recursion_call_result_position() const {
  return get_recursion_parameter_position() - 1;
}

size_t BytecodeCompiler::get_recursion_parameter_position() const {
  return current_offset_ - recursion_parameter_position_;
}

void BytecodeCompiler::compile(const RecursiveFunctionDefinitionNode& node) {
  if (node.use_previous_value) {
    auto zero_case = compile_node(node.zero_case);

    recursion_parameter_position_ = current_offset_ + 3;
    auto general_case = compile_node(node.general_case, 4);

    //
    size_t offset = zero_case.size() + general_case.size();

    result_.splice(result_.end(), zero_case);

    /* 1 */ result_.emplace_back(InstructionType::LOAD, 0);

    // jump to the end
    /* 2 */ result_.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                                 offset + 16);

    // load variables for recursion cycle
    /* 3 */ result_.emplace_back(InstructionType::LOAD, 0);
    /* 4 */ result_.emplace_back(InstructionType::DECREMENT);
    /* 5 */ result_.emplace_back(InstructionType::LOAD_CONST, 0);
    /* 6 */ result_.emplace_back(InstructionType::COPY, 2);

    offset_jumps(general_case, result_.size());
    result_.splice(result_.end(), general_case);

    // leave cycle if we calculated last value
    /* 7 */ result_.emplace_back(InstructionType::POP, 1);
    /* 8 */ result_.emplace_back(InstructionType::COPY, 2);
    /* 9 */ result_.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                                 offset + 13);

    // increment loop counter
    /* 10 */ result_.emplace_back(InstructionType::INCREMENT, 1);
    /* 11 */ result_.emplace_back(InstructionType::DECREMENT, 2);

    // jump back to cycle start
    /* 12 */ result_.emplace_back(InstructionType::LOAD_CONST, 0);
    /* 13 */ result_.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                                  zero_case.size() + 7);

    /* 14 */ result_.emplace_back(InstructionType::POP, 1);
    /* 15 */ result_.emplace_back(InstructionType::POP, 1);
    /* 16 */ result_.emplace_back(InstructionType::POP, 1);
  } else {
    auto zero_case = compile_node(node.zero_case, 1);

    recursion_parameter_position_ = current_offset_ + 1;
    auto general_case = compile_node(node.general_case, 1);

    //
    result_.emplace_back(InstructionType::LOAD, 0);
    result_.emplace_back(InstructionType::JUMP_IF_NONZERO,
                         zero_case.size() + 4);

    offset_jumps(zero_case, result_.size());
    result_.splice(result_.end(), zero_case);

    result_.emplace_back(InstructionType::LOAD_CONST, 0);
    result_.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                         general_case.size() + zero_case.size() + 6);

    result_.emplace_back(InstructionType::DECREMENT, 0);

    offset_jumps(general_case, result_.size());
    result_.splice(result_.end(), general_case);

    result_.emplace_back(InstructionType::POP, 1);
  }
}
}  // namespace Compilation