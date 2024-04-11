#ifndef BYTECODEPRINTER_H
#define BYTECODEPRINTER_H
#include <concepts>
#include <iostream>
#include <string>
#include <vector>

#include "Instructions.h"

using std::vector, std::string;

class BytecodePrinter {
  template <typename Head, typename... Tail>
    requires std::convertible_to<Head, string>
  static string concatenate(Head head, Tail... tail) {
    return string(head) + " " + concatenate(tail...);
  }

  template <typename Head, typename... Tail>
  static string concatenate(Head head, Tail... tail) {
    return std::to_string(head) + " " + concatenate(tail...);
  }

  static string concatenate() { return ""; }

  static string get_instruction_representation(
      const Compilation::Instruction& instruction) {
    switch (instruction.type) {
      case Compilation::InstructionType::INCREMENT:
        return concatenate("INCREMENT", instruction.argument);
      case Compilation::InstructionType::DECREMENT:
        return concatenate("DECREMENT", instruction.argument);
      case Compilation::InstructionType::POP_JUMP_IF_ZERO:
        return concatenate("POP_JUMP_IF_ZERO", instruction.argument);
      case Compilation::InstructionType::JUMP_IF_NONZERO:
        return concatenate("JUMP_IF_NONZERO", instruction.argument);
      case Compilation::InstructionType::CALL_FUNCTION:
        return "CALL_FUNCTION";
      case Compilation::InstructionType::LOAD:
        return concatenate("LOAD", instruction.argument);
      case Compilation::InstructionType::LOAD_CONST:
        return concatenate("LOAD_CONST", instruction.argument);
      case Compilation::InstructionType::LOAD_CALL:
        return concatenate("LOAD_CALL", instruction.argument);
      case Compilation::InstructionType::COPY:
        return concatenate("COPY", instruction.argument);
      case Compilation::InstructionType::RETURN:
        return "RETURN";
      case Compilation::InstructionType::POP:
        return concatenate("POP", instruction.argument);
      case Compilation::InstructionType::HALT:
        return "HALT";
    }

    return "UNKNOWN";
  }

 public:
  static void print(const vector<Compilation::Instruction>& instructions) {
    for (size_t line_id = 0; line_id < instructions.size(); ++line_id) {
      std::cout << line_id << ":\t"
                << get_instruction_representation(instructions[line_id])
                << std::endl;
    }
  }
};

#endif  // BYTECODEPRINTER_H
