#ifndef FASTADD_H
#define FASTADD_H

#include <list>

#include "compilation/Instructions.h"

using std::list;

namespace Compilation {
// clang-format off
inline list<Instruction> fast_add_instructions = {
  {InstructionType::LOAD, 0},
  {InstructionType::POP_JUMP_IF_ZERO, 8},
  {InstructionType::LOAD, 1},
  {InstructionType::LOAD, 0},
  {InstructionType::INCREMENT, 1},
  {InstructionType::DECREMENT, 0},
  {InstructionType::JUMP_IF_NONZERO, 4},
  {InstructionType::POP_JUMP_IF_ZERO, 9},
  {InstructionType::LOAD, 1},
};
// clang-format on
}  // namespace Compilation

#endif  // FASTADD_H
