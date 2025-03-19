#include "LivenessAnalysis.h"

#include <utility>

void Passes::LivenessAnalysis::perform_analysis(const IR::Program& program) {
  phi_values.clear();
  start_dfa(program);
}

std::unordered_map<IR::Value, bool> Passes::LivenessAnalysis::meet(
    std::span<IR::BasicBlock* const> children, const IR::BasicBlock& current) {
  std::unordered_map<IR::Value, bool> result;

  auto values =
      storage_.get_data(children.front(), Position::BEFORE) | std::views::keys;

  for (IR::Value value : values) {
    bool is_live = std::ranges::any_of(
        children, [this, value, &current](IR::BasicBlock* const child) {
          auto phi_values_itr = phi_values.find(std::pair{value, child});

          if (phi_values_itr != phi_values.end() &&
              phi_values_itr->second != &current) {
            return false;
          }

          return storage_.get_data(child, Position::BEFORE).at(value);
        });

    result.emplace(value, is_live);
  }

  return result;
}

std::unordered_map<IR::Value, bool> Passes::LivenessAnalysis::transfer(
    const std::unordered_map<IR::Value, bool>& after,
    const IR::BaseInstruction& instruction, const IR::BasicBlock& current) {
  auto result = after;

  if (instruction.has_return_value()) {
    result[instruction.get_return_value()] = false;
  }

  for (auto temp :
       instruction.filter_arguments(IR::ValueType::VIRTUAL_REGISTER)) {
    result[temp] = true;
  }

  if (instruction.is_of_type<IR::Phi>()) {
    auto& phi = static_cast<const IR::Phi&>(instruction);

    for (auto& [parent, value] : phi.parents) {
      phi_values.emplace(std::pair{value, &current}, parent);
    }
  }

  return result;
}

void Passes::LivenessAnalysis::init(const IR::Function& function) {
  std::unordered_map<IR::Value, bool> init_value;
  for (auto temp : function.temporaries) {
    init_value.emplace(temp, false);
  }

  for (auto& block : function.basic_blocks) {
    for (auto& instruction : block.instructions) {
      storage_.get_data(instruction.get(), Position::BEFORE) = init_value;
      storage_.get_data(instruction.get(), Position::AFTER) = init_value;
    }
  }
}
