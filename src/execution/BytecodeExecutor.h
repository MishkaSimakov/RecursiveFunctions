#ifndef BYTECODEEXECUTOR_H
#define BYTECODEEXECUTOR_H

#include <array>
#include <list>
#include <vector>

#include "compilation/Instructions.h"

using std::array, std::vector, std::pair;

class BytecodeExecutor {
  constexpr static bool kAssertions = true;

  constexpr static size_t kCallStackSize = 1'000;
  constexpr static size_t kValuesStackSize = 1'000;
  constexpr static size_t kMaxIterations = 1'000'000;

  array<pair<size_t, size_t>, kCallStackSize> call_stack_;
  array<ValueT, kValuesStackSize> call_arguments_stack_{};
  array<ValueT, kValuesStackSize> calculation_stack_{};

 public:
  ValueT execute(const vector<Compilation::Instruction>& instructions) {
    int command_ptr = 0;
    size_t call_stack_ptr = 0;
    size_t call_arguments_stack_ptr = 0;
    size_t calculation_stack_ptr = 0;
    bool finished = false;

    for (size_t iteration = 0; iteration < kMaxIterations; ++iteration) {
      auto command = instructions[command_ptr];

      switch (command.type) {
        case Compilation::InstructionType::INCREMENT:
          ++calculation_stack_[calculation_stack_ptr - command.first_argument -
                               1];
          break;
        case Compilation::InstructionType::DECREMENT:
          --calculation_stack_[calculation_stack_ptr - command.first_argument -
                               1];
          break;
        case Compilation::InstructionType::JUMP_IF_ZERO:
          if (calculation_stack_[calculation_stack_ptr - 1] == 0) {
            command_ptr = command.first_argument;
            --command_ptr;
          }
          break;
        case Compilation::InstructionType::JUMP_IF_NONZERO:
          if (calculation_stack_[calculation_stack_ptr - 1] != 0) {
            command_ptr = command.first_argument;
            --command_ptr;
          }
          break;
        case Compilation::InstructionType::CALL_FUNCTION:
          // copy arguments to call arguments stack
          for (size_t i = 0; i < command.second_argument; ++i) {
            call_arguments_stack_[call_arguments_stack_ptr] =
                calculation_stack_[--calculation_stack_ptr];
            ++call_arguments_stack_ptr;
          }

          call_stack_[call_stack_ptr] = {command_ptr, command.second_argument};
          ++call_stack_ptr;
          command_ptr = command.first_argument - 1;
          break;
        case Compilation::InstructionType::LOAD:
          calculation_stack_[calculation_stack_ptr] =
              call_arguments_stack_[call_arguments_stack_ptr -
                                    command.first_argument - 1];
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::LOAD_CONST:
          calculation_stack_[calculation_stack_ptr] = command.first_argument;
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::RETURN:
          --call_stack_ptr;
          call_arguments_stack_ptr -= call_stack_[call_stack_ptr].second;
          command_ptr = call_stack_[call_stack_ptr].first;

          break;
        case Compilation::InstructionType::POP:
          --calculation_stack_ptr;
          break;
        case Compilation::InstructionType::HALT:
          finished = true;
          break;
      }

      if (finished) {
        return calculation_stack_[calculation_stack_ptr - 1];
      }

      ++command_ptr;
    }

    throw std::runtime_error("Iterations limit was reached while executing.");
  }
};

#endif  // BYTECODEEXECUTOR_H
