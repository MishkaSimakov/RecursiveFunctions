#include "DebugBytecodeExecutor.h"

void DebugBytecodeExecutor::execute_instruction(Instruction instruction) {
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
          calculation_stack_[calculation_stack_ptr - 1 - instruction.argument];
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
      command_ptr = calculation_stack_[calculation_stack_ptr].as_line_id() - 1;
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
      execution_status_ = ExecutionStatus::FINISHED;
      break;
  }
}

void DebugBytecodeExecutor::breakpoint_command(size_t position) {
  if (position >= instructions_.size()) {
    fmt::print(
        "Could not set breakpoint on line {}. You can set breakpoint on lines "
        "from 0 to {}",
        position, instructions_.size());
    return;
  }

  breakpoints_.insert(position);
}

void DebugBytecodeExecutor::run_command() {
  execution_mode_ = ExecutionMode::EXECUTE_UNTIL_BREAKPOINT;
  execution_status_ = ExecutionStatus::EXECUTING;
}

void DebugBytecodeExecutor::list_command(size_t line) const {
  print_program_vicinity(line);
}

void DebugBytecodeExecutor::help_command() {
  std::cout << "this is help." << std::endl;
}

void DebugBytecodeExecutor::print_stack_command(StackSlice slice) const {
  using StackType = StackSlice::StackType;

  size_t stack_ptr = 0;
  size_t stack_size = 0;

  switch (slice.stack) {
    case StackType::CALL_STACK:
      stack_ptr = call_stack_ptr;
      stack_size = kCallStackSize;
      break;
    case StackType::ARGUMENTS_STACK:
      stack_ptr = call_arguments_stack_ptr;
      stack_size = kValuesStackSize;
      break;
    case StackType::CALCULATIONS_STACK:
      stack_ptr = calculation_stack_ptr;
      stack_size = kValuesStackSize;
      break;
  }

  auto [from, to] = slice.range.value_or(
      pair{stack_ptr > kStackPrintRadius ? stack_ptr - kStackPrintRadius : 0,
           std::min(stack_size, stack_ptr + kStackPrintRadius)});

  if (slice.stack != StackType::CALL_STACK) {
    constexpr size_t kValueWidth = 6;
    int ptr_position = -1;

    string result = "  [ ";
    if (from != 0) {
      result += "... ";
    }

    for (size_t i = from; i <= to; ++i) {
      ValueT value = slice.stack == StackType::CALCULATIONS_STACK
                         ? calculation_stack_[i]
                         : call_arguments_stack_[i];

      string string_value;
      if (value.is_line_id()) {
        string_value += std::to_string(value.as_line_id()) + "()";
      } else {
        string_value += std::to_string(value.as_value());
      }

      string_value.resize(kValueWidth, ' ');
      result += string_value;

      if (i == stack_ptr) {
        ptr_position = result.size() - kValueWidth;
      }
    }

    if (to != stack_size - 1) {
      result += "... ";
    }

    result += "]\n";

    if (ptr_position != -1) {
      result += string(ptr_position, ' ') + "↑\n";
    }

    std::cout << result << std::flush;
  }
}

void DebugBytecodeExecutor::step_command() {
  execution_mode_ = ExecutionMode::EXECUTE_STEP_BY_STEP;
  execution_status_ = ExecutionStatus::EXECUTING;
}

void DebugBytecodeExecutor::setup_parser() {
  parser_.add_command<size_t>("breakpoint",
                              &DebugBytecodeExecutor::breakpoint_command);
  parser_.add_command<size_t>("list", &DebugBytecodeExecutor::list_command);
  parser_.add_command("help", &DebugBytecodeExecutor::help_command);
  parser_.add_command("run", &DebugBytecodeExecutor::run_command);
  parser_.add_command<StackSlice>("print",
                                  &DebugBytecodeExecutor::print_stack_command);
  parser_.add_command("step", &DebugBytecodeExecutor::step_command);
}

void DebugBytecodeExecutor::print_program_vicinity(size_t center) const {
  size_t start_line =
      center < kProgramPrintRadius ? 0 : center - kProgramPrintRadius;
  size_t end_line =
      std::min(instructions_.size() - 1, center + kProgramPrintRadius);

  for (size_t line = start_line; line <= end_line; ++line) {
    string line_prefix;
    line_prefix += breakpoints_.contains(line) ? "•" : " ";
    if (line == command_ptr) {
      line_prefix += "→";
    }

    fmt::println("{:4}. {:2} {}", line, line_prefix, instructions_[line]);
  }
}

ValueT DebugBytecodeExecutor::execute() {
  Logger::execution(LogLevel::INFO, "start executing bytecode in debug mode");

  std::cout
      << "Entering interactive mode. Interpreter will wait for your commands"
      << std::endl;

  while (execution_status_ != ExecutionStatus::FINISHED) {
    string command;
    std::getline(cin, command);

    parser_.parse(command);

    while (execution_status_ == ExecutionStatus::EXECUTING) {
      auto instruction = instructions_[command_ptr];

      try {
        execute_instruction(instruction);
        ++command_ptr;
      } catch (const StackOutOfBoundsException& exception) {
        // TODO: write about problem
      }

      if (execution_status_ == ExecutionStatus::FINISHED) {
        break;
      }

      ++iteration_;

      if (iteration_ > kMaxIterations) {
        throw std::runtime_error(
            "Iterations limit was reached while executing.");
      }

      size_t current_line = command_ptr;
      if (execution_mode_ == ExecutionMode::EXECUTE_STEP_BY_STEP ||
          (execution_mode_ == ExecutionMode::EXECUTE_UNTIL_BREAKPOINT &&
           breakpoints_.contains(current_line))) {
        execution_status_ = ExecutionStatus::PAUSED;
        print_program_vicinity(command_ptr);

        break;
      }
    }
  }

  Logger::execution(LogLevel::INFO, "successfully executed bytecode");
  Logger::execution(LogLevel::INFO, "execution took {} iterations", iteration_);

  return calculation_stack_[calculation_stack_ptr - 1];
}
