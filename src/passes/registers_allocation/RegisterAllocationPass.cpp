#include "RegisterAllocationPass.h"

#include <Constants.h>

#include <iostream>
#include <numeric>
#include <ranges>
#include <unordered_set>

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

      for (auto* dep : info->dependencies) {
        if (dep->assigned_register.has_value() &&
            dep->assigned_register.value() == value) {
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
  // if function arguments were assigned to different registers we must first
  // move them into correct registers

  for (auto& block : function.basic_blocks) {
    for (auto& instruction : block.instructions) {
    }
  }
}

void Passes::RegisterAllocationPass::apply() {
  for (auto& function : manager_.program.functions) {
    // std::cout << function.name << std::endl;

    // first we determine which temporaries are basic and which are
    // callee-saved:
    std::unordered_map<IR::Value, TemporaryInfo> calls;

    for (auto temp : function.temporaries_info | std::views::keys) {
      calls.emplace(temp, TemporaryInfo{});

      for (auto& basic_block : function.basic_blocks) {
        for (auto& instruction : basic_block.instructions) {
          auto& before = manager_.live_storage.get_live(
              instruction.get(), LiveTemporariesStorage::Position::BEFORE);
          auto& after = manager_.live_storage.get_live(
              instruction.get(), LiveTemporariesStorage::Position::AFTER);

          auto* call = dynamic_cast<const IR::FunctionCall*>(instruction.get());

          if (call == nullptr) {
            continue;
          }

          if (before.contains(temp) && after.contains(temp)) {
            calls[temp].inside_lifetime.push_back(call);
          }

          if (before.contains(temp) || after.contains(temp)) {
            calls[temp].on_lifetime_edge.push_back(call);
          }
        }
      }
    }

    // then for basic registers we must find preferred registers (because they
    // can be used in function calls and return values)
    for (auto& [temp, info] : calls) {
      if (!info.is_basic()) {
        continue;
      }

      // for function argument we increase usage count for appropriate register
      if (temp.value < function.arguments_count) {
        ++info.registers_usage[temp.value];
      }

      for (auto call : info.on_lifetime_edge) {
        if (call->return_value == temp) {
          ++info.registers_usage[0];
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
    for (auto& basic_block : function.basic_blocks) {
      for (auto& instruction : basic_block.instructions) {
        auto& before = manager_.live_storage.get_live(
            instruction.get(), LiveTemporariesStorage::Position::BEFORE);
        auto& after = manager_.live_storage.get_live(
            instruction.get(), LiveTemporariesStorage::Position::AFTER);

        auto create_dependencies =
            [&calls](const std::unordered_set<IR::Value>& tmps) {
              for (auto s = tmps.begin(); s != tmps.end(); ++s) {
                for (auto e = std::next(s); e != tmps.end(); ++e) {
                  calls[*s].dependencies.insert(&calls[*e]);
                }
              }
            };

        create_dependencies(before);
        create_dependencies(after);
      }
    }

    // then we must find best suitable registers for basic case
    std::vector<std::pair<IR::Value, TemporaryInfo*>> basic_tmps_info;
    std::vector<std::pair<IR::Value, TemporaryInfo*>> callee_saved_tmps_info;

    for (auto& [temp, info] : calls) {
      if (info.is_basic()) {
        basic_tmps_info.emplace_back(temp, &info);
      } else {
        callee_saved_tmps_info.emplace_back(temp, &info);
      }
    }

    assign_registers(basic_tmps_info, IR::ValueType::BASIC_REGISTER);
    assign_registers(callee_saved_tmps_info,
                     IR::ValueType::CALLEE_SAVED_REGISTER);

    // for (auto& [temp, info] : calls) {
    //   std::cout << temp.to_string() << " "
    //             << (info.assigned_register.has_value()
    //                     ? std::to_string(info.assigned_register.value())
    //                     : "-")
    //             << std::endl;
    // }

    apply_transformation(function, calls);
  }
}
