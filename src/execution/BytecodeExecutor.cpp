#include "BytecodeExecutor.h"

ValueT BytecodeExecutor::execute(
    const vector<Compilation::Instruction>& instructions) {
  Logger::execution(LogLevel::INFO, "start executing bytecode");

  int command_ptr = 0;
  size_t call_stack_ptr = 0;
  size_t call_arguments_stack_ptr = 0;
  size_t calculation_stack_ptr = 0;
  bool finished = false;

  for (size_t iteration = 0; iteration < kMaxIterations; ++iteration) {
    auto command = instructions[command_ptr];

    switch (command.type) {
      case Compilation::InstructionType::POP_JUMP_IF_ZERO:
        if (calculation_stack_[--calculation_stack_ptr] == 0) {
          command_ptr = command.argument - 1;
        }

        break;
      case Compilation::InstructionType::DECREMENT:
        calculation_stack_[calculation_stack_ptr - command.argument - 1]
            .decrement();
        break;
      case Compilation::InstructionType::LOAD_CONST:
        calculation_stack_[calculation_stack_ptr] =
            ValueT::construct_value(command.argument);
        ++calculation_stack_ptr;
        break;
      case Compilation::InstructionType::JUMP_IF_NONZERO:
        if (calculation_stack_[calculation_stack_ptr - 1] != 0) {
          command_ptr = command.argument - 1;
        }
        break;
      case Compilation::InstructionType::COPY:
        calculation_stack_[calculation_stack_ptr] =
            calculation_stack_[calculation_stack_ptr - 1 - command.argument];
        ++calculation_stack_ptr;
        break;
      case Compilation::InstructionType::LOAD:
        calculation_stack_[calculation_stack_ptr] =
            call_arguments_stack_[call_arguments_stack_ptr - command.argument -
                                  1];
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
            ValueT::construct_line_id(command.argument);
        ++calculation_stack_ptr;
        break;

      case Compilation::InstructionType::RETURN:
        --call_stack_ptr;
        call_arguments_stack_ptr -= call_stack_[call_stack_ptr].second;
        command_ptr = call_stack_[call_stack_ptr].first;

        break;
      case Compilation::InstructionType::INCREMENT:
        calculation_stack_[calculation_stack_ptr - command.argument - 1]
            .increment();
        break;
      case Compilation::InstructionType::POP:
        for (size_t i = command.argument; i != 0; --i) {
          calculation_stack_[calculation_stack_ptr - i - 1] =
              calculation_stack_[calculation_stack_ptr - i];
        }
        --calculation_stack_ptr;
        break;
      case Compilation::InstructionType::HALT:
        finished = true;
        break;
    }

    if (finished) [[unlikely]] {
      Logger::execution(LogLevel::INFO, "successfully executed bytecode");
      Logger::execution(LogLevel::INFO, "execution took {} iterations",
                        iteration);

      return calculation_stack_[calculation_stack_ptr - 1];
    }

    ++command_ptr;
  }

  throw std::runtime_error("Iterations limit was reached while executing.");
}
