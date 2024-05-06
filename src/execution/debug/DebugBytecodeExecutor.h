#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <functional>
#include <set>
#include <vector>

#include "CommandParser.h"
#include "SafeArray.h"
#include "compilation/Instructions.h"
#include "log/Logger.h"

using Compilation::Instruction;
using std::array, std::vector, std::pair, std::set;

enum class ContainerType { CALL, CALL_ARGUMENTS, CALCULATION, INSTRUCTIONS };

struct ContainerOutOfBoundsException final : std::exception {
  string message;

  static string get_container_name(ContainerType type) {
    switch (type) {
      case ContainerType::CALL:
        return "call stack";
      case ContainerType::CALL_ARGUMENTS:
        return "call arguments stack";
      case ContainerType::CALCULATION:
        return "calculations stack";
      case ContainerType::INSTRUCTIONS:
        return "instructions storage";
    }

    return "unknown stack type";
  }

  static string get_violation_name(ArrayBordersViolationType violation) {
    switch (violation) {
      case ArrayBordersViolationType::BORDERS_OVERFLOW:
        return "overflow";
      case ArrayBordersViolationType::BORDERS_UNDERFLOW:
        return "underflow";
    }

    return "unknown violation type";
  }

  ContainerOutOfBoundsException(ContainerType stack, int index,
                                ArrayBordersViolationType violation)
      : message(fmt::format(
            "Container {} happened during program execution. Program "
            "attempted "
            "to use element of {} with index {} which is out of bounds",
            get_violation_name(violation), get_container_name(stack), index)) {}

  const char* what() const noexcept override { return message.c_str(); }
};

template <ContainerType S>
struct OutOfBoundsThrower {
  ContainerOutOfBoundsException operator()(
      int index, ArrayBordersViolationType violation) const {
    return {S, index, violation};
  }
};

class DebugBytecodeExecutor {
  // constants
  constexpr static size_t kProgramPrintRadius = 5;
  constexpr static size_t kStackPrintRadius = 10;

  constexpr static size_t kCallStackSize = 1e5;
  constexpr static size_t kValuesStackSize = 1e5;
  constexpr static size_t kMaxIterations = 1e11;

  const static map<StackSlice::StackType, std::function<string(size_t)>>
      kStackPrinters;

  // types
  enum class ExecutionMode {
    EXECUTE_UNTIL_BREAKPOINT,
    EXECUTE_STEP_BY_STEP,
    EXECUTE_UNTIL_LINE
  };

  enum class ExecutionStatus { EXECUTING, PAUSED, FINISHED };

  using CallStackT = SafeWrapper<array<pair<size_t, size_t>, kCallStackSize>,
                                 OutOfBoundsThrower<ContainerType::CALL>>;
  using ArgumentsStackT =
      SafeWrapper<array<ValueT, kValuesStackSize>,
                  OutOfBoundsThrower<ContainerType::CALL_ARGUMENTS>>;
  using CalculationsStackT =
      SafeWrapper<array<ValueT, kValuesStackSize>,
                  OutOfBoundsThrower<ContainerType::CALCULATION>>;

  using InstructionsContainerT =
      SafeWrapper<vector<Instruction>,
                  OutOfBoundsThrower<ContainerType::INSTRUCTIONS>>;

  // variables

  InstructionsContainerT instructions_;

  size_t iteration_ = 0;
  int command_ptr = 0;
  int call_stack_ptr = 0;
  int call_arguments_stack_ptr = 0;
  int calculation_stack_ptr = 0;

  // information for debug
  CommandParser<DebugBytecodeExecutor> parser_;

  ExecutionStatus execution_status_ = ExecutionStatus::PAUSED;
  ExecutionMode execution_mode_ = ExecutionMode::EXECUTE_UNTIL_BREAKPOINT;
  set<size_t> breakpoints_;

  CallStackT call_stack_{};
  ArgumentsStackT call_arguments_stack_{};
  CalculationsStackT calculation_stack_{};

  void execute_instruction(Instruction instruction);

  void breakpoint_command(size_t position);
  void run_command();
  void list_command(size_t line) const;
  void help_command();
  void print_stack_command(StackSlice slice) const;
  void step_command();

  void setup_parser();

  void print_program_vicinity(size_t center) const;

 public:
  explicit DebugBytecodeExecutor(vector<Instruction> instructions)
      : instructions_(std::move(instructions)), parser_(this) {
    setup_parser();
  }

  ValueT execute();
};
