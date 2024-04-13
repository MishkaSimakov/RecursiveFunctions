#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <list>

#include "concatenate.h"

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
  CALL_RECURSIVE,
  LOAD,
  LOAD_CONST,
  LOAD_CALL,
  COPY,
  RETURN,
  POP,
  HALT
};

struct Instruction {
  InstructionType type;
  size_t argument = 0;

  Instruction(InstructionType type, size_t argument)
      : type(type), argument(argument) {}

  explicit Instruction(InstructionType type) : Instruction(type, 0) {}
};

enum class BlockState { COMPLETED, ONLY_ZERO_CASE, ONLY_GENERAL_CASE };

struct Block {
  list<Instruction> instructions;

  explicit Block(list<Instruction> instructions)
      : instructions(std::move(instructions)) {}
};
}  // namespace Compilation

inline std::ostream& operator<<(
    std::ostream& os, const Compilation::InstructionType instruction_type) {
  switch (instruction_type) {
    case Compilation::InstructionType::INCREMENT:
      return os << "INCREMENT";
    case Compilation::InstructionType::DECREMENT:
      return os << "DECREMENT";
    case Compilation::InstructionType::POP_JUMP_IF_ZERO:
      return os << "POP_JUMP_IF_ZERO";
    case Compilation::InstructionType::JUMP_IF_NONZERO:
      return os << "JUMP_IF_NONZERO";
    case Compilation::InstructionType::CALL_FUNCTION:
      return os << "CALL_FUNCTION";
    case Compilation::InstructionType::CALL_RECURSIVE:
      return os << "CALL_RECURSIVE";
    case Compilation::InstructionType::LOAD:
      return os << "LOAD";
    case Compilation::InstructionType::LOAD_CONST:
      return os << "LOAD_CONST";
    case Compilation::InstructionType::LOAD_CALL:
      return os << "LOAD_CALL";
    case Compilation::InstructionType::COPY:
      return os << "COPY";
    case Compilation::InstructionType::RETURN:
      return os << "RETURN";
    case Compilation::InstructionType::POP:
      return os << "POP";
    case Compilation::InstructionType::HALT:
      return os << "HALT";
    default:
      return os << "UNKNOWN";
  }
}

inline std::ostream& operator<<(std::ostream& os,
                                const Compilation::Instruction instruction) {
  using Compilation::InstructionType;

  os << instruction.type;

  if (instruction.type != InstructionType::RETURN &&
      instruction.type != InstructionType::HALT &&
      instruction.type != InstructionType::CALL_FUNCTION) {
    os << " " << instruction.argument;
  }

  return os;
}

#endif  // INSTRUCTIONS_H
