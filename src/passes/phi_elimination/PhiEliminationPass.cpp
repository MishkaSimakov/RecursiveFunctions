#include "PhiEliminationPass.h"

#include <iostream>
#include "intermediate_representation/Function.h"
#include "passes/PassManager.h"

bool Passes::PhiEliminationPass::apply(IR::Function& function) {
  bool was_changed = false;

  for (auto& block : function.basic_blocks) {
    auto& instructions = block.instructions;

    for (auto itr = instructions.begin(); itr != instructions.end();) {
      auto* phi_node = dynamic_cast<const IR::Phi*>(itr->get());

      if (phi_node == nullptr) {
        ++itr;
        continue;
      }

      auto phi_temporary = function.allocate_vreg();

      std::cout << phi_node->to_string() << " " << phi_temporary.to_string() << std::endl;

      for (auto [parent, value] : phi_node->parents) {
        auto insert_itr = std::prev(parent->instructions.end());

        parent->instructions.insert(
            insert_itr, std::make_unique<IR::Move>(phi_temporary, value));
      }

      *itr = std::make_unique<IR::Move>(phi_node->return_value, phi_temporary);

      was_changed = true;
    }
  }

  return was_changed;
}
