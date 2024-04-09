#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

using ValueT = size_t;

namespace Compilation {
// Инструкции байт-кода. Все, кроме RETURN, принимают 1 аргумент.
enum class InstructionType {
  INCREMENT,
  DECREMENT,
  JUMP_IF_ZERO,
  JUMP_IF_NONZERO,
  CALL_FUNCTION,
  LOAD,
  LOAD_CONST,
  RETURN,
  POP,
  HALT
};

struct Instruction {
  InstructionType type;
  size_t first_argument = 0;
  size_t second_argument = 0;

  Instruction(InstructionType type, size_t first = 0, size_t second = 0)
      : type(type), first_argument(first), second_argument(second) {}
};
}

#endif  // INSTRUCTIONS_H
