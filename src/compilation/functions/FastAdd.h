#ifndef FASTADD_H
#define FASTADD_H

#include <list>

#include "compilation/Instructions.h"

using std::list;

namespace Compilation {
// clang-format off
inline list<Instruction> fast_add_instructions = {
  {InstructionType::LOAD, 0},
  {InstructionType::JUMP_IF_ZERO, 10},
  {InstructionType::POP},
  {InstructionType::LOAD, 1},
  {InstructionType::LOAD, 0},
  {InstructionType::INCREMENT, 1},
  {InstructionType::DECREMENT, 0},
  {InstructionType::JUMP_IF_NONZERO, 5},
  {InstructionType::POP},
  {InstructionType::RETURN},
  {InstructionType::POP},
  {InstructionType::LOAD, 1},
  {InstructionType::RETURN}
};
// clang-format on
}  // namespace Compilation

#endif  // FASTADD_H
