#pragma once

#include <fmt/core.h>

#include <iostream>
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

  std::string as_string() const {
    if (is_line_id()) {
      return std::to_string(as_line_id()) + "()";
    }

    return std::to_string(as_value());
  }

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

template <>
struct fmt::formatter<Compilation::Instruction> {
 private:
  static auto stringify_instruction_type(Compilation::InstructionType type) {
    switch (type) {
      case Compilation::InstructionType::INCREMENT:
        return "INCREMENT";
      case Compilation::InstructionType::DECREMENT:
        return "DECREMENT";
      case Compilation::InstructionType::POP_JUMP_IF_ZERO:
        return "POP_JUMP_IF_ZERO";
      case Compilation::InstructionType::JUMP_IF_NONZERO:
        return "JUMP_IF_NONZERO";
      case Compilation::InstructionType::CALL_FUNCTION:
        return "CALL_FUNCTION";
      case Compilation::InstructionType::LOAD:
        return "LOAD";
      case Compilation::InstructionType::LOAD_CONST:
        return "LOAD_CONST";
      case Compilation::InstructionType::LOAD_CALL:
        return "LOAD_CALL";
      case Compilation::InstructionType::COPY:
        return "COPY";
      case Compilation::InstructionType::RETURN:
        return "RETURN";
      case Compilation::InstructionType::POP:
        return "POP";
      case Compilation::InstructionType::HALT:
        return "HALT";
      default:
        return "UNKNOWN";
    }
  }

 public:
  constexpr auto parse(auto& ctx) { return ctx.begin(); }

  auto format(const Compilation::Instruction& instruction, auto& ctx) const {
    using Compilation::InstructionType;

    InstructionType type = instruction.type;
    bool without_argument = type == InstructionType::RETURN ||
                            type == InstructionType::HALT ||
                            type == InstructionType::CALL_FUNCTION;

    if (without_argument) {
      return fmt::format_to(ctx.out(), "{}", stringify_instruction_type(type));
    }

    return fmt::format_to(ctx.out(), "{} {}", stringify_instruction_type(type),
                          instruction.argument);
  }
};

inline std::ostream& operator<<(std::ostream& os,
                                const Compilation::Instruction instruction) {
  fmt::print("{}", instruction);
  return os;
}
