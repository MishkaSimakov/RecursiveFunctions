#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

namespace Compilation {
// Инструкции байт-кода. Все, кроме RETURN, принимают 1 аргумент.
enum class Instructions {
  INCREMENT,
  DECREMENT,
  JUMP_IF_ZERO,
  CALL_FUNCTION,
  LOAD,
  LOAD_CONST,
  RETURN,
  POP
};
}

#endif  // INSTRUCTIONS_H
