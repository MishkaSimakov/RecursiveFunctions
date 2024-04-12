#ifndef FASTABSOLUTEDIFFERENCE_H
#define FASTABSOLUTEDIFFERENCE_H

#include <list>

#include "compilation/Instructions.h"

using std::list;

namespace Compilation {
// clang-format off
inline list<Instruction> fast_absolute_difference_instructions = {
  {InstructionType::LOAD, 0},
  {InstructionType::LOAD, 1},
  {InstructionType::JUMP_IF_NONZERO, 5}, // if first is zero
  {InstructionType::POP},
  {InstructionType::RETURN},
  {InstructionType::COPY, 1},
  {InstructionType::POP_JUMP_IF_ZERO, 11}, // if second is zero

  // 7: if none of them zero
  {InstructionType::DECREMENT},
  {InstructionType::DECREMENT, 1},

  // 9: repeat
  {InstructionType::LOAD_CONST, 0},
  {InstructionType::POP_JUMP_IF_ZERO, 2},

  // 11: if second is zero
  {InstructionType::POP, 1},
  {InstructionType::RETURN},
};
// clang-format on
}  // namespace Compilation

#endif  // FASTABSOLUTEDIFFERENCE_H
