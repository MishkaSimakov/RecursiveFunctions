#include "CostModel.h"

void Passes::CostModel::process_function_call(const IR::FunctionCall& call) {
  auto used = call.filter_arguments(IR::ValueType::VIRTUAL_REGISTER);

  // every live temporary before call must be saved into heap
  for (auto temp :
       liveness.get_live(&call, LiveTemporariesStorage::Position::BEFORE)) {
    costs[temp].basic += kLoadStoreCost;
  }

  // and after call loaded from heap
  for (auto temp :
       liveness.get_live(&call, LiveTemporariesStorage::Position::AFTER)) {
    costs[temp].basic += kLoadStoreCost;
  }

  // arguments must be moved
  for (size_t i = 0; i < function.arguments_count; ++i) {
    if (call.arguments[i].type == IR::ValueType::CONSTANT) {
      continue;
    }
  }

  // return value must be moved from return-value-register
}

std::unordered_map<IR::Value, Passes::CostModel::Costs>
Passes::CostModel::estimate_costs() {
  // this simple cost model will only count instructions that must be added, not
  // their location

  for (auto& block : function.basic_blocks) {
    for (auto& instruction : block.instructions) {
      // we load used temporaries before each instruction
      for (auto temp :
           instruction->filter_arguments(IR::ValueType::VIRTUAL_REGISTER)) {
        costs[temp].spill += kLoadStoreCost;
      }

      if (instruction->is_of_type<IR::FunctionCall>()) {
        process_function_call(
            static_cast<const IR::FunctionCall&>(*instruction));
      } else if (instruction->has_return_value()) {
        // result might be saved into heap
        costs[instruction->get_return_value()].spill += kLoadStoreCost;
      }
    }
  }
}
