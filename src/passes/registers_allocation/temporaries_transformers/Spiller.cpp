#include "Spiller.h"

#include <iostream>
#include <unordered_map>

#include "passes/registers_allocation/RegistersInfo.h"

void Passes::Spiller::process_function_call(
    const std::unordered_set<IR::Value>& temporaries, IR::Function& function,
    IR::BasicBlock& block,
    IR::BasicBlock::InstructionsListT::iterator instruction_itr) {}

void Passes::Spiller::spill(IR::Function& function,
                            const std::unordered_set<IR::Value>& temporaries) {
  std::unordered_map<IR::Value, IR::Value> stack_indices;

  for (auto temp : temporaries) {
    stack_indices.emplace(
        temp, IR::Value(stack_indices.size(), IR::ValueType::STACK_INDEX));
  }

  // we should store function arguments if they were spilled
  for (auto argument: function.arguments) {
    if (stack_indices.contains(argument)) {
      function.begin_block->instructions.push_front(
          std::make_unique<IR::Store>(argument, stack_indices[argument]));
    }
  }

  for (auto& block : function.basic_blocks) {
    for (auto itr = block.instructions.begin(); itr != block.instructions.end();
         ++itr) {
      auto& instruction = *itr;

      if (instruction->is_of_type<IR::FunctionCall>()) {
        process_function_call(temporaries, function, block, itr);

        continue;
      }

      auto used_temporaries =
          instruction->filter_arguments(IR::ValueType::VIRTUAL_REGISTER);

      auto temp_registers = RegistersInfo::kSpillTemporaryRegisters;
      std::unordered_map<IR::Value, IR::Value> replacement_map;

      for (auto temp : used_temporaries) {
        if (!temporaries.contains(temp)) {
          continue;
        }

        // we must spill that temporary

        if (temp_registers.empty()) {
          // TODO: add support for loading more than three (maybe using SWP
          // instruction)
          std::cout << instruction->to_string() << std::endl;

          throw std::runtime_error(
              "Loading of more than three temporaries from stack for one "
              "instruction is not supported yet!");
        }

        auto next_temp_register =
            IR::Value(*temp_registers.begin(), IR::ValueType::VIRTUAL_REGISTER);
        temp_registers.erase(next_temp_register.value);
        replacement_map[temp] = next_temp_register;

        block.instructions.insert(
            itr, std::make_unique<IR::Load>(next_temp_register,
                                            stack_indices[temp]));
      }

      if (instruction->has_return_value() &&
          temporaries.contains(instruction->get_return_value())) {
        IR::Value result_temp = instruction->get_return_value();
        IR::Value index = stack_indices[result_temp];

        replacement_map[instruction->get_return_value()] = IR::Value(
            RegistersInfo::kReturnRegister, IR::ValueType::VIRTUAL_REGISTER);

        block.instructions.insert(
            std::next(itr), std::make_unique<IR::Store>(
                                IR::Value(RegistersInfo::kReturnRegister,
                                          IR::ValueType::VIRTUAL_REGISTER),
                                index));
      }

      instruction->replace_values(replacement_map);
    }
  }
}
