#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <list>

using std::list;

struct ValueT {
 private:
  ValueT(size_t value) : value(value) {}

 public:
  constexpr static size_t kFunctionCallOffset = 1 << 31;
  size_t value;

  ValueT() : value(0) {}

  static ValueT construct_line_id(size_t line_id) {
    return line_id | kFunctionCallOffset;
  }

  static ValueT construct_value(size_t value) { return value; }

  bool is_line_id() const { return (value & kFunctionCallOffset) != 0; }

  size_t as_value() const { return value; }

  size_t as_line_id() const { return value & ~kFunctionCallOffset; }

  void increment() { ++value; }

  void decrement() { --value; }

  bool operator==(size_t other) const { return value == other; }
};

namespace Compilation {
enum class InstructionType {
  INCREMENT,
  DECREMENT,
  POP_JUMP_IF_ZERO,
  JUMP_IF_NONZERO,
  CALL_FUNCTION,
  LOAD,
  LOAD_CONST,
  LOAD_CALL,
  RETURN,
  POP,
  HALT
};

struct Instruction {
  InstructionType type;
  size_t argument = 0;

  Instruction(InstructionType type, size_t argument = 0)
      : type(type), argument(argument) {}
};

enum class BlockState { COMPLETED, ONLY_ZERO_CASE, ONLY_GENERAL_CASE };

struct Block {
  list<Instruction> instructions;

  explicit Block(list<Instruction> instructions)
      : instructions(std::move(instructions)) {}
};
}  // namespace Compilation

#endif  // INSTRUCTIONS_H
