#include "LivenessAnalysis.h"

void Passes::LivenessAnalysis::perform_analysis(const IR::Program& program) {
  start_dfa(program);
}

std::unordered_map<IR::Value, Passes::TemporaryLivenessState>
Passes::LivenessAnalysis::meet(
    std::vector<const std::unordered_map<IR::Value, TemporaryLivenessState>*>
        children) const {
  std::unordered_map<IR::Value, TemporaryLivenessState> result;

  for (auto value : *children.front() | std::views::keys) {
    int state =
        std::ranges::max(children | std::views::transform([value](auto& map) {
                           return static_cast<int>(map->at(value));
                         }));

    result.emplace(value, static_cast<TemporaryLivenessState>(state));
  }

  return result;
}

std::unordered_map<IR::Value, Passes::TemporaryLivenessState>
Passes::LivenessAnalysis::transfer(
    const std::unordered_map<IR::Value, TemporaryLivenessState>& after,
    const IR::BaseInstruction& instruction) const {
  auto result = after;

  if (instruction.has_return_value()) {
    result[instruction.get_return_value()] = TemporaryLivenessState::DEAD;
  }

  for (auto temp :
       instruction.filter_arguments(IR::ValueType::VIRTUAL_REGISTER)) {
    result[temp] = TemporaryLivenessState::LIVE;
  }

  return result;
}

void Passes::LivenessAnalysis::init(const IR::Function& function) {
  std::unordered_map<IR::Value, TemporaryLivenessState> init_value;
  for (auto temp : function.temporaries) {
    init_value.emplace(temp, TemporaryLivenessState::TOP);
  }

  for (auto& block : function.basic_blocks) {
    for (auto& instruction : block.instructions) {
      storage_.get_data(instruction.get(), Position::BEFORE) = init_value;
      storage_.get_data(instruction.get(), Position::AFTER) = init_value;
    }
  }
}
