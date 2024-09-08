#include "LoopSymbolicAnalysis.h"

#include <iostream>

#include "passes/PassManager.h"
#include "passes/analysis/dominators/DominatorsAnalysis.h"

namespace Passes {
namespace {
class LinearFunction {
  int get_coef_for_key(IR::Value key) const {
    auto itr = parts.find(key);

    if (itr != parts.end()) {
      return itr->second;
    }

    return 0;
  }

 public:
  constexpr static auto kConstKey = IR::Value(0, IR::ValueType::CONSTANT);
  constexpr static auto kIterationKey = IR::Value(1, IR::ValueType::CONSTANT);

  // variable/coefficient
  // constant stored as value=#0 and coefficient=123123
  std::unordered_map<IR::Value, int> parts;

  LinearFunction() = default;

  explicit LinearFunction(IR::Value value) {
    if (value.type == IR::ValueType::CONSTANT) {
      parts[kConstKey] = value.value;
    } else {
      parts[value] = 1;
    }
  }

  bool is_constant() const {
    return parts.size() == 1 && parts.contains(kConstKey);
  }

  int get_constant_part() const { return get_coef_for_key(kConstKey); }

  int get_variable_coef(IR::Value variable) const {
    return get_coef_for_key(variable);
  }

  LinearFunction& operator*=(int scalar) {
    if (scalar == 0) {
      parts.clear();
      return *this;
    }

    for (auto& [value, coef] : parts) {
      coef *= scalar;
    }

    return *this;
  }

  LinearFunction& operator+=(const LinearFunction& other) {
    if (&other == this) {
      *this *= 2;
      return *this;
    }

    for (auto& [value, coef] : other.parts) {
      parts[value] += coef;
    }

    return *this;
  }

  LinearFunction& operator-=(const LinearFunction& other) {
    if (&other == this) {
      *this *= 2;
      return *this;
    }

    for (auto& [value, coef] : other.parts) {
      parts[value] -= coef;
    }

    return *this;
  }

  LinearFunction& operator-() {
    *this *= -1;
    return *this;
  }
};

LinearFunction operator+(const LinearFunction& left,
                         const LinearFunction& right) {
  LinearFunction copy = left;
  copy += right;
  return copy;
}

LinearFunction operator-(const LinearFunction& left,
                         const LinearFunction& right) {
  LinearFunction copy = left;
  copy -= right;
  return copy;
}

LinearFunction operator*(const LinearFunction& left, int scalar) {
  LinearFunction copy = left;
  copy *= scalar;
  return copy;
}

std::ostream& operator<<(std::ostream& os, const LinearFunction& func) {
  if (func.parts.empty()) {
    os << "0";
    return os;
  }

  auto parts =
      func.parts |
      std::views::transform([](const std::pair<IR::Value, int>& part) {
        if (part.first == LinearFunction::kConstKey) {
          return std::to_string(part.second);
        }

        if (part.second == 1) {
          return part.first.to_string();
        }
        if (part.second == -1) {
          return "-" + part.first.to_string();
        }

        return std::to_string(part.second) + "*" + part.first.to_string();
      });

  os << fmt::format("{}", fmt::join(parts, " + "));
  return os;
}

struct LoopVariableInfo {
  bool is_valid = true;
  LinearFunction transfer_polynom;
  LinearFunction initial_value;

  //
  LinearFunction nth_iteration;

  void iterate(IR::Value variable) {
    // for now we support iterations of function of type: variable + b
    for (auto [value, coef] : transfer_polynom.parts) {
      if (value != LinearFunction::kConstKey && value != variable) {
        return;
      }
    }

    int variable_coef = transfer_polynom.get_variable_coef(variable);
    int constant_coef = transfer_polynom.get_constant_part();
  }
};

class LoopAnalysisVisitor : public IR::InstructionVisitor {
 private:
  LoopVariableInfo& get_info(IR::Value value) {
    auto [itr, _] = variable_info.emplace(
        value,
        LoopVariableInfo{true, LinearFunction(value), LinearFunction(value)});

    return itr->second;
  }

 public:
  std::unordered_map<IR::Value, LoopVariableInfo>& variable_info;
  const IR::BasicBlock& loop_block;

  LoopAnalysisVisitor(
      std::unordered_map<IR::Value, LoopVariableInfo>& variable_info,
      const IR::BasicBlock& loop_block)
      : variable_info(variable_info), loop_block(loop_block) {}

  void visit(const IR::FunctionCall&) override {}
  void visit(const IR::Addition& instruction) override {
    auto& result_info = variable_info[instruction.return_value];
    const auto& left_info = get_info(instruction.arguments[0]);
    const auto& right_info = get_info(instruction.arguments[1]);

    result_info.is_valid = left_info.is_valid && right_info.is_valid;
    if (result_info.is_valid) {
      result_info.transfer_polynom =
          left_info.transfer_polynom + right_info.transfer_polynom;
      result_info.initial_value =
          left_info.initial_value + right_info.initial_value;
    }
  }

  void visit(const IR::Subtraction& instruction) override {
    auto& result_info = variable_info[instruction.return_value];
    const auto& left_info = get_info(instruction.arguments[0]);
    const auto& right_info = get_info(instruction.arguments[1]);

    result_info.is_valid = left_info.is_valid && right_info.is_valid;
    if (result_info.is_valid) {
      result_info.transfer_polynom =
          left_info.transfer_polynom - right_info.transfer_polynom;
      result_info.initial_value =
          left_info.initial_value - right_info.initial_value;
    }
  }

  void visit(const IR::Move& instruction) override {
    auto value = instruction.arguments[0];
    auto& info = variable_info[instruction.return_value];

    if (value.type == IR::ValueType::CONSTANT) {
      info.is_valid = true;
      info.transfer_polynom = LinearFunction(value);
      info.initial_value = LinearFunction(value);
    } else {
      variable_info[instruction.return_value] =
          variable_info[instruction.arguments[0]];
    }
  }

  void visit(const IR::Phi& instruction) override {
    auto entry_value = instruction.parents[0].second;
    auto backedge_value = instruction.parents[1].second;

    if (instruction.parents[0].first == &loop_block) {
      std::swap(entry_value, backedge_value);
    }

    auto& info = variable_info[instruction.return_value];

    info.initial_value = LinearFunction(entry_value);
    info.transfer_polynom = LinearFunction(backedge_value);
  }

  void visit(const IR::Return&) override {
    throw std::runtime_error("Return in loop?");
  }
  void visit(const IR::Branch&) override {
    // branch doesn't change values
  }

  void visit(const IR::Jump&) override {
    throw std::runtime_error("Jump in loop?");
  }

  void visit(const IR::Load& instruction) override {
    variable_info[instruction.return_value].is_valid = false;
  }

  void visit(const IR::Store& instruction) override {}

  void visit(const IR::Select& instruction) override {
    // TODO: add support for select
    variable_info[instruction.return_value].is_valid = false;
  }
};
}  // namespace
}  // namespace Passes

void Passes::LoopSymbolicAnalysis::perform_analysis(
    const IR::Program& program) {
  for (auto& function : program.functions) {
    std::cout << function.name << std::endl;

    std::cout << LinearFunction(IR::Value(10, IR::ValueType::VIRTUAL_REGISTER))
              << "\n";

    auto loops =
        manager_.get_analysis<DominatorsAnalysis>().get_loops(function);

    for (auto& loop : loops) {
      // for now we only analyse simple one-block loops
      if (loop.blocks.size() != 1) {
        continue;
      }

      auto* block = *loop.blocks.begin();

      // only one entry to loop and one back edge
      if (block->parents.size() != 2) {
        continue;
      }

      std::cout << "New loop" << std::endl;

      std::unordered_map<IR::Value, LoopVariableInfo> variables_info;
      LoopAnalysisVisitor visitor(variables_info, *block);

      for (auto& instruction : block->instructions) {
        instruction->accept(visitor);
      }

      for (auto& [variable, info] : variables_info) {
        if (!info.is_valid || variable.type == IR::ValueType::CONSTANT) {
          continue;
        }

        info.iterate(variable);

        std::cout << "\t" << variable.to_string() << ":\n";
        std::cout << "\t\t"
                  << "Transfer: " << info.transfer_polynom << "\n";
        std::cout << "\t\t"
                  << "Initial: " << info.initial_value << "\n";
        std::cout << "\t\t"
                  << "Iteration: " << info.nth_iteration << "\n";
      }
    }
  }
}
