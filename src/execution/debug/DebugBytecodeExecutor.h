#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <set>
#include <vector>
#include <fmt/core.h>

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

#ifdef __cpp_explicit_this_parameter
  auto& operator[](this auto&& self, int index) {
    if (index < 0 || index >= Size) {
      throw self.thrower_(index);
    }

    return self.storage_[index];
  }
#else
  auto& operator[](int index) {
    if (index < 0 || index >= Size) {
      throw thrower_(index);
    }

    return storage_[index];
  }

  const auto& operator[](int index) const {
    return const_cast<SafeArray&>(*this)[index];
  }
#endif
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

  enum class ExecutionStatus { EXECUTING, PAUSED, FINISHED };

  constexpr static size_t kProgramPrintRadius = 5;
  constexpr static size_t kStackPrintRadius = 10;

  constexpr static size_t kCallStackSize = 1e5;
  constexpr static size_t kValuesStackSize = 1e5;
  constexpr static size_t kMaxIterations = 1e11;

  vector<Instruction> instructions_;

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

  SafeArray<pair<size_t, size_t>, kCallStackSize, OutOfBoundsThrower<0>>
      call_stack_;
  SafeArray<ValueT, kValuesStackSize, OutOfBoundsThrower<1>>
      call_arguments_stack_{};
  SafeArray<ValueT, kValuesStackSize, OutOfBoundsThrower<2>>
      calculation_stack_{};

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
  DebugBytecodeExecutor(vector<Instruction> instructions)
      : instructions_(std::move(instructions)), parser_(this) {
    setup_parser();
  }

  ValueT execute();
};