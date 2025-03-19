#include "VerificationPass.h"

#include "passes/analysis/liveness/LivenessAnalysis.h"

bool Passes::VerificationPass::apply(IR::Program& program) {
  auto& liveness =
      manager_.get_analysis<LivenessAnalysis>().get_liveness_info();

  for (auto& function : program.functions) {
    if (manager_.is_in_ssa()) {
      check_ssa(function);
    }

    for (auto& block : function.basic_blocks) {
      // if there is only one child it must be the first one in pair
      if (block.children[0] == nullptr && block.children[1] != nullptr) {
        throw VerificationException(function, "Wrong children ordering");
      }

      // empty blocks are banned
      if (block.instructions.empty()) {
        throw VerificationException(function, "No empty blocks");
      }

      // each block must end with control flow instruction
      if (!block.instructions.back()->is_control_flow_instruction()) {
        throw VerificationException(
            function,
            "Every block should have control flow instruction at the end");
      }

      // there must be only one control flow instruction
      for (auto& instruction : block.instructions) {
        if (instruction->is_control_flow_instruction() &&
            instruction != block.instructions.back()) {
          throw VerificationException(
              function,
              "There must be only one control flow instruction in each block");
        }

        if (instruction->is_of_type<IR::Phi>()) {
          auto& phi = static_cast<const IR::Phi&>(*instruction);

          if (phi.parents.size() != block.parents.size()) {
            throw VerificationException(
                function, fmt::format("Phi instruction \"{}\" doesn't "
                                      "enumerate all parents",
                                      instruction->to_string()));
          }

          auto& alive = liveness.get_data(instruction.get(), Position::AFTER);
          for (auto [parent, value] : phi.parents) {
            if (alive.contains(value) && alive.at(value)) {
              throw VerificationException(
                  function,
                  fmt::format("Value {} is not assigned before usage", value));
            }
          }

          std::unordered_set<const IR::BasicBlock*> phi_parents;
          for (auto parent : phi.parents | std::views::keys) {
            phi_parents.insert(parent);
          }

          for (auto parent : block.parents) {
            if (!phi_parents.contains(parent)) {
              throw VerificationException(
                  function,
                  "Phi instruction must contain only current block parents");
            }
          }
        }
      }
    }

    // only function arguments must be live before first instruction
    auto& before = liveness.get_data(
        function.begin_block->instructions.front().get(), Position::BEFORE);
    auto live_before =
        before | std::views::filter([](const std::pair<IR::Value, bool>& pair) {
          return pair.second;
        }) |
        std::views::keys;

    for (auto value : live_before) {
      if (std::ranges::find(function.arguments, value) ==
          function.arguments.end()) {
        throw VerificationException(
            function, fmt::format("Value {} is used but has path that "
                                  "does not define this value.",
                                  value));
      }
    }
  }

  return false;
}

void Passes::VerificationPass::check_ssa(const IR::Function& function) {
  std::unordered_set<IR::Value> defined;

  auto define_value_with_check = [&defined, &function](IR::Value value) {
    auto [itr, was_inserted] = defined.emplace(value);

    if (!was_inserted) {
      throw VerificationException(
          function, fmt::format("SSA form is not satisfied. Value {} has more "
                                "than one definition.",
                                value));
    }
  };

  for (auto& argument : function.arguments) {
    define_value_with_check(argument);
  }

  for (auto& block : function.basic_blocks) {
    for (auto& instruction : block.instructions) {
      if (instruction->has_return_value()) {
        define_value_with_check(instruction->get_return_value());
      }
    }
  }
}
