#include "VerificationPass.h"

#include "passes/analysis/liveness/LivenessAnalysis.h"

bool Passes::VerificationPass::apply(IR::Program& program) {
  for (auto& function : program.functions) {
    for (auto& block : function.basic_blocks) {
      // if there is only one child it must be the first one in pair
      if (block.children[0] == nullptr && block.children[1] != nullptr) {
        throw std::runtime_error("Wrong children ordering");
      }

      // empty blocks are banned
      if (block.instructions.empty()) {
        throw std::runtime_error("No empty blocks");
      }

      // each block must end with control flow instruction
      if (!block.instructions.back()->is_control_flow_instruction()) {
        throw std::runtime_error(
            "Every block should have control flow instruction at the end");
      }

      // there must be only one control flow instruction
      for (auto& instruction : block.instructions) {
        if (instruction->is_control_flow_instruction() &&
            instruction != block.instructions.back()) {
          throw std::runtime_error(
              "There must be only one control flow instruction in each block");
        }

        if (instruction->is_of_type<IR::Phi>()) {
          auto& phi = static_cast<const IR::Phi&>(*instruction);

          if (phi.parents.size() != block.parents.size()) {
            throw std::runtime_error(
                "Each phi instruction must contain all block parents");
          }

          std::unordered_set<const IR::BasicBlock*> phi_parents;
          for (auto parent : phi.parents | std::views::keys) {
            phi_parents.insert(parent);
          }

          for (auto parent : block.parents) {
            if (!phi_parents.contains(parent)) {
              throw std::runtime_error(
                  "Phi instruction must contain only current block parents");
            }
          }
        }
      }
    }

    auto& liveness =
        manager_.get_analysis<LivenessAnalysis>().get_liveness_info();

    // only function arguments must be live before first instruction
    auto& before = liveness.get_data(
        function.begin_block->instructions.front().get(), Position::BEFORE);
    auto live_before =
        before |
        std::views::filter(
            [](const std::pair<IR::Value, TemporaryLivenessState>& pair) {
              return pair.second == TemporaryLivenessState::LIVE;
            }) |
        std::views::keys;

    for (auto value : live_before) {
      if (std::ranges::find(function.arguments, value) ==
          function.arguments.end()) {
        throw std::runtime_error(
            fmt::format("In function \"{}\" value {} is used but has path that "
                        "does not define this value.",
                        function.name, value));
      }
    }
  }

  return false;
}
