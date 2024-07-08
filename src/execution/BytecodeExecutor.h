#pragma once

#include <algorithm>
#include <array>
#include <vector>

#include "compilation/Instructions.h"
#include "log/Logger.h"

using std::array, std::vector, std::pair;

class BytecodeExecutor {
  constexpr static bool kAssertions = true;

  constexpr static size_t kCallStackSize = 1e5;
  constexpr static size_t kValuesStackSize = 1e5;
  constexpr static size_t kDefaultMaxIterations = 1e11;

  size_t max_iterations = kDefaultMaxIterations;

  array<pair<size_t, size_t>, kCallStackSize> call_stack_;
  array<ValueT, kValuesStackSize> call_arguments_stack_{};
  array<ValueT, kValuesStackSize> calculation_stack_{};

 public:
  void set_iterations_limit(size_t limit) { max_iterations = limit; }

  ValueT execute(const vector<Compilation::Instruction>& instructions);
};
