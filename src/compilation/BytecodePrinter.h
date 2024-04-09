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

  static string concatenate() {
    return "";
  }

  static string get_instruction_representation(
      const Compilation::Instruction& instruction) {
    switch (instruction.type) {
      case Compilation::InstructionType::INCREMENT:
        return concatenate("INCREMENT", instruction.first_argument);
      case Compilation::InstructionType::DECREMENT:
        return concatenate("DECREMENT", instruction.first_argument);
      case Compilation::InstructionType::JUMP_IF_ZERO:
        return concatenate("JUMP_IF_ZERO", instruction.first_argument);
      case Compilation::InstructionType::JUMP_IF_NONZERO:
        return concatenate("JUMP_IF_NONZERO", instruction.first_argument);
      case Compilation::InstructionType::CALL_FUNCTION:
        return concatenate("CALL_FUNCTION", instruction.first_argument,
                           instruction.second_argument);
      case Compilation::InstructionType::LOAD:
        return concatenate("LOAD", instruction.first_argument);
      case Compilation::InstructionType::LOAD_CONST:
        return concatenate("LOAD_CONST", instruction.first_argument);
      case Compilation::InstructionType::RETURN:
        return "RETURN";
      case Compilation::InstructionType::POP:
        return "POP";
      case Compilation::InstructionType::HALT:
        return "HALT";
    }

    return "UNKNOWN";
  }

 public:
  static void print(const vector<Compilation::Instruction>& instructions) {
    for (auto& instruction : instructions) {
      std::cout << get_instruction_representation(instruction) << std::endl;
    }
  }
};

#endif  // BYTECODEPRINTER_H
