#ifndef BYTECODEEXECUTOR_H
#define BYTECODEEXECUTOR_H

#include <log/Logger.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <vector>

#include "compilation/Instructions.h"

// enables collection of time-consuming statistics
#define COLLECT_STATISTICS false

using std::array, std::vector, std::pair, std::unordered_map;
using namespace std::chrono_literals;

struct ExecutorStatistics {
  size_t iterations_count = 0;
  unordered_map<Compilation::InstructionType, size_t>
      instructions_interations{};
  std::chrono::duration<double> execution_time = 0ms;

  size_t max_call_stack_usage = 0;
  size_t max_call_arguments_stack_usage = 0;
  size_t max_calculation_stack_usage = 0;

  bool is_ready = false;

  void update_call_stack_usage(size_t usage) {
    if (usage > max_call_stack_usage) {
      max_call_stack_usage = usage;
    }
  }

  void update_call_arguments_stack_usage(size_t usage) {
    if (usage > max_call_arguments_stack_usage) {
      max_call_arguments_stack_usage = usage;
    }
  }

  void update_calculation_stack_usage(size_t usage) {
    if (usage > max_calculation_stack_usage) {
      max_calculation_stack_usage = usage;
    }
  }
};

class BytecodeExecutor {
  constexpr static bool kAssertions = true;

  constexpr static size_t kCallStackSize = 1e5;
  constexpr static size_t kValuesStackSize = 1e5;
  constexpr static size_t kMaxIterations = 1e11;

  array<pair<size_t, size_t>, kCallStackSize> call_stack_;
  array<ValueT, kValuesStackSize> call_arguments_stack_{};
  array<ValueT, kValuesStackSize> calculation_stack_{};

  ExecutorStatistics statistics_;

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
    Logger::execution(LogLevel::INFO, "start executing bytecode");

    auto start_time = std::chrono::steady_clock::now();

    int command_ptr = 0;
    size_t call_stack_ptr = 0;
    size_t call_arguments_stack_ptr = 0;
    size_t calculation_stack_ptr = 0;
    bool finished = false;

    for (size_t iteration = 0; iteration < kMaxIterations; ++iteration) {
    start:
      auto command = instructions[command_ptr];

#if COLLECT_STATISTICS
      ++statistics_.instructions_interations[command.type];
      statistics_.update_call_stack_usage(call_stack_ptr - call_stack_.begin());
      statistics_.update_call_arguments_stack_usage(
          call_arguments_stack_ptr - call_arguments_stack_.begin());
      statistics_.update_calculation_stack_usage(calculation_stack_ptr -
                                                 calculation_stack_.begin());
#endif

      switch (command.type) {
        case Compilation::InstructionType::POP_JUMP_IF_ZERO:
          if (calculation_stack_[--calculation_stack_ptr] == 0) {
            command_ptr = command.argument;
            goto start;
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
            command_ptr = command.argument;
            goto start;
          }
          break;
        case Compilation::InstructionType::COPY:
          calculation_stack_[calculation_stack_ptr] =
              calculation_stack_[calculation_stack_ptr - 1 - command.argument];
          ++calculation_stack_ptr;
          break;
        case Compilation::InstructionType::LOAD:
          calculation_stack_[calculation_stack_ptr] =
              call_arguments_stack_[call_arguments_stack_ptr -
                                    command.argument - 1];
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
        default:
          throw std::runtime_error("Unimplemented command!");
      }

      if (finished) [[unlikely]] {
        statistics_.iterations_count = iteration;
        statistics_.execution_time =
            std::chrono::steady_clock::now() - start_time;
        statistics_.is_ready = true;

        size_t seconds_cnt = std::chrono::duration_cast<std::chrono::seconds>(
                                 statistics_.execution_time)
                                 .count();

        Logger::execution(LogLevel::INFO, "successfully executed bytecode");
        Logger::execution(LogLevel::INFO, "execution took", seconds_cnt,
                          "seconds", "and", statistics_.iterations_count,
                          "iterations");

        return calculation_stack_[calculation_stack_ptr - 1];
      }

      ++command_ptr;
    }

    throw std::runtime_error("Iterations limit was reached while executing.");
  }

#if COLLECT_STATISTICS
  void print_statistics(std::ostream& os) const {
    using std::endl;

    os << "Total iterations: " << statistics_.iterations_count << endl;
    os << "By instruction type:" << endl;

    vector<pair<size_t, Compilation::InstructionType>> sorted_instructions;
    for (auto& [instruction, iterations] :
         statistics_.instructions_interations) {
      sorted_instructions.emplace_back(iterations, instruction);
    }

    std::sort(sorted_instructions.begin(), sorted_instructions.end(),
              std::greater());

    for (auto& [iterations, instruction] : sorted_instructions) {
      os << " - " << std::setw(25) << instruction << ":\t"
         << std::setprecision(2)
         << (100 * static_cast<double>(iterations) /
             statistics_.iterations_count)
         << "%";

      os << "\t(" << iterations << ")" << endl;
    }

    os << endl;
    os << "Stacks usage:" << endl;
    os << " - " << std::setw(25) << "Call stack:\t"
       << statistics_.max_call_stack_usage << endl;
    os << " - " << std::setw(25) << "Call arguments stack:\t"
       << statistics_.max_call_arguments_stack_usage << endl;
    os << " - " << std::setw(25) << "Calculations stack:\t"
       << statistics_.max_calculation_stack_usage << endl;

    os << endl;
    os << "In overall execution took: " << statistics_.execution_time.count()
       << "ms" << endl;
  }
#endif

  auto get_execution_duration() const { return statistics_.execution_time; }
};

#endif  // BYTECODEEXECUTOR_H