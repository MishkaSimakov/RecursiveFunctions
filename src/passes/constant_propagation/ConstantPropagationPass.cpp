// #include "ConstantPropagationPass.h"
//
// #include <iostream>
// #include <unordered_set>
//
// #include "InstructionValueCalculator.h"
// #include "intermediate_representation/BasicBlock.h"
// #include "passes/PassManager.h"
//
// Passes::ForwardDFA<std::unordered_map<IR::Value, IR::Value>>::DFAValueT
// Passes::ConstantPropagationPass::meet(std::span<DFAValueT> parents) const {
//   DFAValueT result;
//
//   for (const auto& parent_info : parents) {
//     for (auto [temp, value] : parent_info) {
//       auto [itr, was_inserted] = result.emplace(temp, value);
//
//       if (!was_inserted) {
//         itr->second = join_abstract_values(itr->second, value);
//       }
//     }
//   }
//
//   return result;
// }
//
// bool Passes::ConstantPropagationPass::transfer(
//     const IR::BaseInstruction* instruction) {
//   if (!instruction->has_return_value()) {
//     return false;
//   }
//
//   auto return_value = instruction->get_return_value();
//
//   auto& info_before = values.get_data(instruction, Position::BEFORE);
//   auto& info_after = values.get_data(instruction, Position::AFTER);
//   auto old_value = info_after[return_value];
//
//   info_after.clear();
//
//   auto instruction_copy = instruction->clone();
//   instruction_copy->replace_values(info_before);
//   auto new_value = InstructionValueCalculator().calculate(*instruction_copy);
//
//   info_after[return_value] = new_value;
//
//   return old_value == new_value;
// }
//
// bool Passes::ConstantPropagationPass::process_block(const IR::BasicBlock* block,
//                                                     bool stop_on_unchanged) {
//   bool was_changed = meet(block);
//
//   if (!was_changed && stop_on_unchanged) {
//     return false;
//   }
//
//   auto end = block->instructions.end();
//   for (auto itr = block->instructions.begin(); itr != end; ++itr) {
//     // transfer through instruction (before -> after)
//     was_changed = transfer(itr->get());
//
//     if (!was_changed && stop_on_unchanged) {
//       return false;
//     }
//
//     // transfer between instructions (after -> before)
//     if (std::next(itr) != end) {
//       auto current = itr->get();
//       auto next = std::next(itr)->get();
//       values.get_data(next, Position::BEFORE) =
//           values.get_data(current, Position::AFTER);
//     }
//   }
//
//   return true;
// }
//
// IR::Value Passes::ConstantPropagationPass::join_abstract_values(
//     IR::Value first, IR::Value second) {
//   if (first == kBottom || second == kBottom) {
//     return kBottom;
//   }
//
//   if (first.value == second.value) {
//     return first;
//   }
//
//   return kBottom;
// }
//
// void Passes::ConstantPropagationPass::apply() {
//   for (auto& function : manager_.program.functions) {
//     std::unordered_set<const IR::BasicBlock*> worklist;
//     bool stop_on_unchanged = false;
//
//     auto block_transform_lambda =
//         [this, &worklist, &stop_on_unchanged](const IR::BasicBlock* block) {
//           worklist.erase(block);
//           bool was_changed = process_block(block, stop_on_unchanged);
//
//           if (!was_changed) {
//             return;
//           }
//
//           for (auto child : block->children) {
//             if (child != nullptr) {
//               worklist.insert(child);
//             }
//           }
//         };
//
//     // first we traverse our blocks in reversed postorder and fill worklist
//     function.reversed_postorder_traversal(block_transform_lambda);
//
//     // now we continue using worklist
//     stop_on_unchanged = true;
//
//     while (!worklist.empty()) {
//       const IR::BasicBlock* block = *worklist.begin();
//       block_transform_lambda(block);
//     }
//
//     // now all values are calculated
//     std::cout << function.name << std::endl;
//
//     for (auto& block : function.basic_blocks) {
//       for (auto& instruction : block.instructions) {
//         std::cout << "before: ";
//
//         for (auto value :
//              values.get_data(instruction.get(), Position::BEFORE)) {
//           std::cout << fmt::format("({}, {}) ", value.first, value.second);
//         }
//
//         std::cout << std::endl;
//
//         std::cout << instruction->to_string() << std::endl;
//
//         std::cout << "after: ";
//
//         for (auto value : values.get_data(instruction.get(), Position::AFTER)) {
//           std::cout << fmt::format("({}, {}) ", value.first, value.second);
//         }
//
//         std::cout << std::endl;
//       }
//     }
//   }
// }
