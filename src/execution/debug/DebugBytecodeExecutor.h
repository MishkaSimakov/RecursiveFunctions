#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <set>
#include <vector>

#include "CommandParser.h"
#include "compilation/Instructions.h"
#include "log/Logger.h"

using Compilation::Instruction;
using std::array, std::vector, std::pair, std::set;

namespace ph = std::placeholders;

template <typename T, size_t Size, std::invocable<int> ExceptionThrower>
class SafeArray {
  [[no_unique_address]] ExceptionThrower thrower_;
  std::array<T, Size> storage_;

 public:
  SafeArray() = default;

  auto& operator[](this auto&& self, int index) {
    if (index < 0 || index >= Size) {
      throw self.thrower_(index);
    }

    return self.storage_[index];
  }
};

struct StackOutOfBoundsException {
  size_t stack_id;
  int index;
};

template <size_t I>
struct OutOfBoundsThrower {
  StackOutOfBoundsException operator()(int index) const { return {I, index}; }
};

class DebugBytecodeExecutor {
  enum class ExecutionMode {
    EXECUTE_UNTIL_BREAKPOINT,
    EXECUTE_STEP_BY_STEP,
    EXECUTE_UNTIL_LINE
  };

  constexpr static bool kAssertions = true;

  constexpr static size_t kCallStackSize = 1e5;
  constexpr static size_t kValuesStackSize = 1e5;
  constexpr static size_t kMaxIterations = 1e11;

  int command_ptr = 0;
  int call_stack_ptr = 0;
  int call_arguments_stack_ptr = 0;
  int calculation_stack_ptr = 0;
  bool finished = false;

  // information for debug
  CommandParser parser_;

  bool is_paused_ = true;
  ExecutionMode execution_mode_ = ExecutionMode::EXECUTE_UNTIL_BREAKPOINT;
  set<size_t> breakpoints_;

  SafeArray<pair<size_t, size_t>, kCallStackSize, OutOfBoundsThrower<0>>
      call_stack_;
  SafeArray<ValueT, kValuesStackSize, OutOfBoundsThrower<1>>
      call_arguments_stack_{};
  SafeArray<ValueT, kValuesStackSize, OutOfBoundsThrower<2>>
      calculation_stack_{};

  void execute_instruction(Instruction instruction) {
    switch (instruction.type) {
      case Compilation::InstructionType::POP_JUMP_IF_ZERO:
        if (calculation_stack_[--calculation_stack_ptr] == 0) {
          command_ptr = instruction.argument - 1;
        }

        break;
      case Compilation::InstructionType::DECREMENT:
        calculation_stack_[calculation_stack_ptr - instruction.argument - 1]
            .decrement();
        break;
      case Compilation::InstructionType::LOAD_CONST:
        calculation_stack_[calculation_stack_ptr] =
            ValueT::construct_value(instruction.argument);
        ++calculation_stack_ptr;
        break;
      case Compilation::InstructionType::JUMP_IF_NONZERO:
        if (calculation_stack_[calculation_stack_ptr - 1] != 0) {
          command_ptr = instruction.argument - 1;
        }
        break;
      case Compilation::InstructionType::COPY:
        calculation_stack_[calculation_stack_ptr] =
            calculation_stack_[calculation_stack_ptr - 1 -
                               instruction.argument];
        ++calculation_stack_ptr;
        break;
      case Compilation::InstructionType::LOAD:
        calculation_stack_[calculation_stack_ptr] =
            call_arguments_stack_[call_arguments_stack_ptr -
                                  instruction.argument - 1];
        ++calculation_stack_ptr;
        break;
      case Compilation::InstructionType::CALL_FUNCTION:
        call_stack_[call_stack_ptr].second = 0;

        // copy arguments to call arguments stack
        --calculation_stack_ptr;
        while (!calculation_stack_[calculation_stack_ptr].is_line_id()) {
          call_arguments_stack_[call_arguments_stack_ptr] =
              calculation_stack_[calculation_stack_ptr];
          ++call_arguments_stack_ptr;
          --calculation_stack_ptr;

          ++call_stack_[call_stack_ptr].second;  // calculate arguments count
        }

        call_stack_[call_stack_ptr].first = command_ptr;
        ++call_stack_ptr;
        command_ptr =
            calculation_stack_[calculation_stack_ptr].as_line_id() - 1;
        break;

      case Compilation::InstructionType::LOAD_CALL:
        calculation_stack_[calculation_stack_ptr] =
            ValueT::construct_line_id(instruction.argument);
        ++calculation_stack_ptr;
        break;

      case Compilation::InstructionType::RETURN:
        --call_stack_ptr;
        call_arguments_stack_ptr -= call_stack_[call_stack_ptr].second;
        command_ptr = call_stack_[call_stack_ptr].first;

        break;
      case Compilation::InstructionType::INCREMENT:
        calculation_stack_[calculation_stack_ptr - instruction.argument - 1]
            .increment();
        break;
      case Compilation::InstructionType::POP:
        for (size_t i = instruction.argument; i != 0; --i) {
          calculation_stack_[calculation_stack_ptr - i - 1] =
              calculation_stack_[calculation_stack_ptr - i];
        }
        --calculation_stack_ptr;
        break;
      case Compilation::InstructionType::HALT:
        finished = true;
        break;
    }
  }

  void breakpoint_command(size_t position) {
    std::cout << "breakpoint set at " << position << std::endl;
  }

  static void help_command() { std::cout << "this is help." << std::endl; }

  void setup_parser() {
    parser_.add_command<size_t>(
        "breakpoint",
        std::bind(&DebugBytecodeExecutor::breakpoint_command, this, ph::_1));
    parser_.add_command<>("help", help_command);
  }

 public:
  DebugBytecodeExecutor() { setup_parser(); }

  ValueT execute(const vector<Instruction>& instructions) {
    Logger::execution(LogLevel::INFO, "start executing bytecode in debug mode");

    std::cout
        << "Entering interactive mode. Interpreter will wait for your commands"
        << std::endl;

    string command;
    std::cin >> command;
    std::cout << "start parsing" << std::endl;
    parser_.parse(command);

    //
    for (size_t iteration = 0; iteration < kMaxIterations; ++iteration) {
      auto instruction = instructions[command_ptr];

      try {
        execute_instruction(instruction);
      } catch (const StackOutOfBoundsException& exception) {
        // TODO: write about problem
      }

      if (finished) [[unlikely]] {
        Logger::execution(LogLevel::INFO, "successfully executed bytecode");
        Logger::execution(LogLevel::INFO, "execution took", iteration,
                          "iterations");

        return calculation_stack_[calculation_stack_ptr - 1];
      }

      ++command_ptr;
    }

    throw std::runtime_error("Iterations limit was reached while executing.");
  }
};
