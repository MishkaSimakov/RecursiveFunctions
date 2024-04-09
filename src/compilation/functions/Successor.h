#ifndef SUCCESSOR_H
#define SUCCESSOR_H
#include <list>

#include "compilation/Instructions.h"

using std::list;

namespace Compilation {
inline list successor_instructions = {
    Instruction(InstructionType::LOAD, 0),
    Instruction(InstructionType::INCREMENT, 0),
    Instruction(InstructionType::RETURN)
};
}

#endif  // SUCCESSOR_H
