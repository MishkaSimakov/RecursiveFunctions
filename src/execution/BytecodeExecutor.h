#ifndef BYTECODEEXECUTOR_H
#define BYTECODEEXECUTOR_H

#include <array>
#include <list>
#include <vector>

#include "compilation/Instructions.h"
#include "log/Logger.h"

using std::array, std::vector, std::pair;

class BytecodeExecutor {
  constexpr static bool kAssertions = true;

  constexpr static size_t kCallStackSize = 1e5;
  constexpr static size_t kValuesStackSize = 1e5;
  constexpr static size_t kMaxIterations = 1'000'000;

  array<pair<size_t, size_t>, kCallStackSize> call_stack_;
  array<ValueT, kValuesStackSize> call_arguments_stack_{};
  array<ValueT, kValuesStackSize> calculation_stack_{};

 public:
  ValueT execute(const vector<Compilation::Instruction>& instructions) {
    int command_ptr = 0;
    auto call_stack_ptr = call_stack_.begin();
    auto call_arguments_stack_ptr = call_arguments_stack_.begin();
    auto calculation_stack_ptr = calculation_stack_.begin();
    bool finished = false;

    for (size_t iteration = 0; iteration < kMaxIterations; ++iteration) {
      auto command = instructions[command_ptr];

      switch (command.type) {
        case Compilation::InstructionType::INCREMENT:
          Logger::execution("INCREMENT");
          (calculation_stack_ptr - command.argument - 1)->increment();
          break;
        case Compilation::InstructionType::DECREMENT:
          Logger::execution("DECREMENT");
          (calculation_stack_ptr - command.argument - 1)->decrement();
          break;
        case Compilation::InstructionType::JUMP_IF_ZERO:
          Logger::execution("JUMP_IF_ZERO");
          if (*(calculation_stack_ptr - 1) == 0) {
            Logger::execution("JUMPING:", command.argument);
            command_ptr = command.argument;
            --command_ptr;
          }
          break;
        case Compilation::InstructionType::JUMP_IF_NONZERO:
          Logger::execution("JUMP_IF_NONZERO");
          if (*(calculation_stack_ptr - 1) != 0) {
            Logger::execution("JUMPING:", command.argument);
            command_ptr = command.argument;
            --command_ptr;
          }
          break;
        case Compilation::InstructionType::CALL_FUNCTION:
          Logger::execution("CALL_FUNCTION");
          call_stack_ptr->second = 0;

          // copy arguments to call arguments stack
          --calculation_stack_ptr;
          while (!calculation_stack_ptr->is_line_id()) {
            *call_arguments_stack_ptr = *calculation_stack_ptr;
            ++call_arguments_stack_ptr;
            --calculation_stack_ptr;

            ++call_stack_ptr->second;  // calculate arguments count
          }

          Logger::execution("Arguments count:", call_stack_ptr->second);

          call_stack_ptr->first = command_ptr;
          ++call_stack_ptr;
          command_ptr = calculation_stack_ptr->get_line_id() - 1;
          Logger::execution("Execution continues at:", command_ptr);
          --calculation_stack_ptr;  // pop function call from calculation stack
          break;
        case Compilation::InstructionType::LOAD:
          Logger::execution("LOAD");
          *calculation_stack_ptr =
              *(call_arguments_stack_ptr - command.argument - 1);
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::LOAD_CONST:
          Logger::execution("LOAD_CONST");
          *calculation_stack_ptr = ValueT::construct_value(command.argument);
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::LOAD_CALL:
          Logger::execution("LOAD_CALL");
          *calculation_stack_ptr = ValueT::construct_line_id(command.argument);
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::RETURN:
          Logger::execution("RETURN");
          --call_stack_ptr;
          call_arguments_stack_ptr -= call_stack_ptr->second + 1;
          command_ptr = call_stack_ptr->first;

          break;
        case Compilation::InstructionType::POP:
          Logger::execution("POP");
          --calculation_stack_ptr;
          break;
        case Compilation::InstructionType::HALT:
          Logger::execution("HALT");
          finished = true;
          break;
      }

      if (finished) {
        return *(calculation_stack_ptr - 1);
      }

      ++command_ptr;
    }

    throw std::runtime_error("Iterations limit was reached while executing.");
  }
};

#endif  // BYTECODEEXECUTOR_H
