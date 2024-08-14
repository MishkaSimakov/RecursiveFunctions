#include "RegisterAllocationPass.h"

#include <iostream>
#include <numeric>
#include <ranges>
#include <unordered_set>

#include "Disentangler.h"
#include "passes/analysis/liveness/LivenessAnalysis.h"

void Passes::RegisterAllocationPass::assign_registers(
    const std::vector<std::pair<IR::Value, TemporaryInfo*>>& temps_info,
    IR::ValueType register_type) const {
  for (auto& [temp, info] : temps_info) {
    // bigger is better here
    std::array<size_t, kMaxRegistersAllowed> priorities;
    std::iota(priorities.begin(), priorities.end(), 0);

    std::ranges::sort(priorities, [&usage = info->registers_usage](
                                      size_t first, size_t second) {
      return usage[first] > usage[second];
    });

    // now we go through priorities
    for (int register_candidate : priorities) {
      IR::Value value(register_candidate, register_type);

      // we must check that this candidate is available
      bool allowed = true;

      for (auto dep : info->dependencies) {
        auto& dep_info = vregs_info.at(dep);
        if (dep_info.assigned_register.has_value() &&
            dep_info.assigned_register.value() == value) {
          allowed = false;
          break;
        }
      }

      if (!allowed) {
        continue;
      }

      // we can use this register for this temporary
      info->assigned_register = value;

      break;
    }
  }
}

void Passes::RegisterAllocationPass::apply_transformation(
    IR::Function& function,
    const std::unordered_map<IR::Value, TemporaryInfo>& temps_info) {
  std::unordered_map<IR::Value, IR::Value> replacement_map;

  for (auto& [temp, info] : temps_info) {
    if (!info.assigned_register.has_value()) {
      throw std::runtime_error(
          "Register allocation pass failed to assign registers to temporaries");
    }

    replacement_map[temp] = info.assigned_register.value();
  }

  std::cout << function.name << std::endl;
  for (auto [temp, dest] : replacement_map) {
    std::cout << fmt::format("{} -> {}\n", temp, dest);
  }

  for (auto& block : function.basic_blocks) {
    for (auto itr = block.instructions.begin(); itr != block.instructions.end();
         ++itr) {
      auto& instruction = *itr;

      // first, we change every instruction argument
      instruction->replace_values(replacement_map);

      // then if it is function we must move arguments and return value into
      // correct registers (from 0, 1, 2, ... to whatever was assigned to them)
      auto* call = dynamic_cast<IR::FunctionCall*>(instruction.get());

      if (call == nullptr) {
        continue;
      }

      // used to resolve loops in permutation
      std::vector<std::pair<IR::Value, IR::Value>> knot;
      for (size_t i = 0; i < call->arguments.size(); ++i) {
        auto required_register = IR::Value(i, IR::ValueType::BASIC_REGISTER);
        knot.emplace_back(call->arguments[i], required_register);

        call->arguments[i] = required_register;
      }

      auto moves = Disentangler().disentangle(knot, kTemporaryRegister);

      for (auto [from, to] : moves) {
        block.instructions.insert(itr, std::make_unique<IR::Move>(to, from));
      }

      if (call->return_value != kReturnRegister) {
        block.instructions.insert(
            std::next(itr),
            std::make_unique<IR::Move>(call->return_value, kReturnRegister));

        call->return_value = kReturnRegister;
      }
    }
  }

  // we must disentangle function arguments
  std::vector<std::pair<IR::Value, IR::Value>> knot;
  for (auto& argument : function.arguments) {
    IR::Value argument_copy = argument;
    argument.type = IR::ValueType::BASIC_REGISTER;
    knot.emplace_back(argument, replacement_map.at(argument_copy));
  }

  auto moves = Disentangler().disentangle(knot, kTemporaryRegister);

  auto first_instruction_itr = function.begin_block->instructions.begin();
  for (auto [from, to] : moves) {
    function.begin_block->instructions.insert(
        first_instruction_itr, std::make_unique<IR::Move>(to, from));
  }

  // then we must move return value to zero register
  for (auto* basic_block : function.end_blocks) {
    auto& return_instr =
        static_cast<IR::Return&>(*basic_block->instructions.back());

    if (auto& return_value = return_instr.arguments[0];
        return_value != kReturnRegister) {
      basic_block->instructions.insert(
          std::prev(basic_block->instructions.end()),
          std::make_unique<IR::Move>(kReturnRegister, return_value));

      return_value = kReturnRegister;
    }
  }
}

void Passes::RegisterAllocationPass::apply() {
  auto liveness_info =
      manager_.get_analysis<LivenessAnalysis>().get_liveness_info();

  for (auto& function : manager_.program.functions) {
    vregs_info.clear();

    // first we determine which temporaries are basic and which are
    // callee-saved:
    for (auto temp : function.temporaries) {
      TemporaryInfo info;
      info.virtual_register = temp;

      vregs_info.emplace(temp, std::move(info));

      for (auto& basic_block : function.basic_blocks) {
        for (auto& instruction : basic_block.instructions) {
          auto& before = liveness_info.get_live(
              instruction.get(), LiveTemporariesStorage::Position::BEFORE);
          auto& after = liveness_info.get_live(
              instruction.get(), LiveTemporariesStorage::Position::AFTER);

          auto* call = dynamic_cast<const IR::FunctionCall*>(instruction.get());

          if (call == nullptr) {
            continue;
          }

          if (before.contains(temp) && after.contains(temp)) {
            vregs_info.at(temp).inside_lifetime.push_back(call);
          }

          if (before.contains(temp) || after.contains(temp)) {
            vregs_info.at(temp).on_lifetime_edge.push_back(call);
          }
        }
      }
    }

    // increase usage count for return values (they must be stored in zero
    // register)
    for (auto* basic_block : function.end_blocks) {
      const auto& return_instr =
          static_cast<const IR::Return&>(*basic_block->instructions.back());
      IR::Value return_value = return_instr.arguments[0];

      if (return_value.is_temporary()) {
        ++vregs_info.at(return_value).registers_usage[kReturnRegister.value];
      }
    }

    // then for basic registers we must find preferred registers (because they
    // can be used in function calls and return values)
    for (auto& [temp, info] : vregs_info) {
      if (!info.is_basic()) {
        continue;
      }

      // for function argument we increase usage count for appropriate register
      if (temp.value < function.arguments.size()) {
        ++info.registers_usage[temp.value];
      }

      for (auto call : info.on_lifetime_edge) {
        if (call->return_value == temp) {
          ++info.registers_usage[kReturnRegister.value];
          continue;
        }

        for (size_t i = 0; i < call->arguments.size(); ++i) {
          if (call->arguments[i] == temp) {
            ++info.registers_usage[i];
          }
        }
      }
    }

    // for (auto& [temp, info] : calls) {
    //   std::cout << temp.to_string() << " "
    //             << (info.is_basic() ? "basic" : "callee-saved") << std::endl;
    // }

    // find dependencies between temporaries
    auto create_dependencies =
        [this](const std::unordered_set<IR::Value>& tmps) {
          for (auto s = tmps.begin(); s != tmps.end(); ++s) {
            for (auto e = std::next(s); e != tmps.end(); ++e) {
              vregs_info.at(*s).dependencies.insert(*e);
              vregs_info.at(*e).dependencies.insert(*s);
            }
          }
        };

    for (auto& basic_block : function.basic_blocks) {
      for (auto& instruction : basic_block.instructions) {
        auto& before = liveness_info.get_live(
            instruction.get(), LiveTemporariesStorage::Position::BEFORE);
        auto& after = liveness_info.get_live(
            instruction.get(), LiveTemporariesStorage::Position::AFTER);

        create_dependencies(before);
        create_dependencies(after);
      }
    }

    // std::cout << "dependencies" << std::endl;
    // for (auto& [temp, info] : vregs_info) {
    // std::cout << temp.to_string() << ":\n";
    // for (auto dep : info.dependencies) {
    // std::cout << dep.to_string() << " ";
    // }
    // std::cout << std::endl;
    // }

    // then we must find best suitable registers for basic case
    std::vector<std::pair<IR::Value, TemporaryInfo*>> basic_tmps_info;
    std::vector<std::pair<IR::Value, TemporaryInfo*>> callee_saved_tmps_info;

    for (auto& [temp, info] : vregs_info) {
      if (info.is_basic()) {
        basic_tmps_info.emplace_back(temp, &info);
      } else {
        callee_saved_tmps_info.emplace_back(temp, &info);
      }
    }

    assign_registers(basic_tmps_info, IR::ValueType::BASIC_REGISTER);
    assign_registers(callee_saved_tmps_info,
                     IR::ValueType::CALLEE_SAVED_REGISTER);

    apply_transformation(function, vregs_info);

    function.finalize();
  }
}
