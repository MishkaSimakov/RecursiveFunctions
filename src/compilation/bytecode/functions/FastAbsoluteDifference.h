#pragma once

#include <list>

#include "compilation/Instructions.h"

using std::list;

namespace Compilation {
// clang-format off
inline list<Instruction> fast_absolute_difference_instructions = {
  {InstructionType::LOAD, 0},
  {InstructionType::LOAD, 1},
  {InstructionType::JUMP_IF_NONZERO, 4}, // if first is zero
  {InstructionType::POP_JUMP_IF_ZERO, 11},
  {InstructionType::COPY, 1},
  {InstructionType::POP_JUMP_IF_ZERO, 10}, // if second is zero

  // 6: if none of them zero
  {InstructionType::DECREMENT, 0},
  {InstructionType::DECREMENT, 1},

  // 8: repeat
  {InstructionType::LOAD_CONST, 0},
  {InstructionType::POP_JUMP_IF_ZERO, 2},

  // 10: if second is zero
  {InstructionType::POP, 1},
};
// clang-format on
}  // namespace Compilation
