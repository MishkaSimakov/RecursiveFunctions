#ifndef BYTECODEEXECUTOR_H
#define BYTECODEEXECUTOR_H

#include <array>
#include <list>
#include <vector>

#include "compilation/Instructions.h"

using std::array, std::vector, std::pair;

class BytecodeExecutor {
  constexpr static bool kAssertions = true;

  constexpr static size_t kCallStackSize = 1e5;
  constexpr static size_t kValuesStackSize = 1e5;
  constexpr static size_t kMaxIterations = 1e11;

  array<pair<size_t, size_t>, kCallStackSize> call_stack_;
  array<ValueT, kValuesStackSize> call_arguments_stack_{};
  array<ValueT, kValuesStackSize> calculation_stack_{};

  void echo_stack(auto itr, auto curr_itr, size_t length) {
    std::cout << "[";

    size_t traversed = 0;
    for (; traversed < length; ++traversed, ++itr) {
      std::cout << itr->as_value();

      if (itr == curr_itr) {
        std::cout << "*";
      }

      std::cout << ", ";
    }

    std::cout << "]" << std::endl;
  }

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
          (calculation_stack_ptr - command.argument - 1)->increment();
          break;
        case Compilation::InstructionType::DECREMENT:
          (calculation_stack_ptr - command.argument - 1)->decrement();
          break;
        case Compilation::InstructionType::POP_JUMP_IF_ZERO:
          --calculation_stack_ptr;
          if (*calculation_stack_ptr == 0) {
            command_ptr = command.argument;
            --command_ptr;
          }
          break;
        case Compilation::InstructionType::JUMP_IF_NONZERO:
          if (*(calculation_stack_ptr - 1) != 0) {
            command_ptr = command.argument;
            --command_ptr;
          }
          break;
        case Compilation::InstructionType::CALL_FUNCTION:
          call_stack_ptr->second = 0;

          // copy arguments to call arguments stack
          --calculation_stack_ptr;
          while (!calculation_stack_ptr->is_line_id()) {
            *call_arguments_stack_ptr = *calculation_stack_ptr;
            ++call_arguments_stack_ptr;
            --calculation_stack_ptr;

            ++call_stack_ptr->second;  // calculate arguments count
          }

          call_stack_ptr->first = command_ptr;
          ++call_stack_ptr;
          command_ptr = calculation_stack_ptr->as_line_id() - 1;
          break;
        case Compilation::InstructionType::LOAD:
          *calculation_stack_ptr =
              *(call_arguments_stack_ptr - command.argument - 1);
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::LOAD_CONST:
          *calculation_stack_ptr = ValueT::construct_value(command.argument);
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::LOAD_CALL:
          *calculation_stack_ptr = ValueT::construct_line_id(command.argument);
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::COPY:
          *calculation_stack_ptr =
              *(calculation_stack_ptr - 1 - command.argument);
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::RETURN:
          --call_stack_ptr;
          call_arguments_stack_ptr -= call_stack_ptr->second;
          command_ptr = call_stack_ptr->first;

          break;
        case Compilation::InstructionType::POP:
          if (command.argument == 0) {
            --calculation_stack_ptr;
          } else {
            for (size_t i = 0; i < command.argument; ++i) {
              *(calculation_stack_ptr - command.argument + i - 1) =
                  *(calculation_stack_ptr - command.argument + i);
            }
            --calculation_stack_ptr;
          }
          break;
        case Compilation::InstructionType::HALT:
          finished = true;
          break;
        default:
          throw std::runtime_error("Unimplemented command!");
      }

      if (finished) {
        std::cout << iteration << std::endl;
        return *(calculation_stack_ptr - 1);
      }

      // std::cout << "Call arguments stack: ";
      // echo_stack(call_arguments_stack_.begin(), call_arguments_stack_ptr,
      // 20);
      //
      // std::cout << "Calculations stack:   ";
      // echo_stack(calculation_stack_.begin(), calculation_stack_ptr, 20);

      ++command_ptr;
    }

    throw std::runtime_error("Iterations limit was reached while executing.");
  }
};

#endif  // BYTECODEEXECUTOR_H

// 0:	LOAD_CALL 8
// 1:	LOAD_CONST 20
// 2:	LOAD_CONST 10
// 3:	CALL_FUNCTION
// 4:	HALT
// 5:	LOAD 0
// 6:	INCREMENT 0
// 7:	RETURN
// 8:	LOAD 0
// 9:	JUMP_IF_ZERO 18
// 10:	POP
// 11:	LOAD_CALL 5
// 12:	LOAD_CALL 8
// 13:	LOAD 0
// 14:	LOAD 1
// 15:	CALL_FUNCTION
// 16:	CALL_FUNCTION
// 17:	RETURN
// 18:	POP
// 19:	LOAD 1
// 20:	RETURN