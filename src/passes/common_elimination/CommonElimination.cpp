#include "CommonElimination.h"

#include <iostream>

#include "intermediate_representation/Function.h"

void Passes::CommonElimination::process_block(IR::Function& function,
                                              IR::BasicBlock& block) {
  auto& instr = block.instructions;
  for (auto first = instr.begin(); first != instr.end(); ++first) {
    for (auto second = std::next(first); second != instr.end();) {
      if (**first == **second) {
        IR::Value return_value = (*second)->get_return_value();
        IR::Value original_return_value = (*first)->get_return_value();

        second = block.instructions.erase(second);

        std::unordered_map<IR::Value, IR::Value> replacement_map;
        replacement_map.emplace(return_value, original_return_value);
        function.replace_values(replacement_map);
      } else {
        ++second;
      }
    }
  }
}
