#include "RecursionToLoopPass.h"

#include <ranges>

#include "passes/PassManager.h"
#include "passes/analysis/liveness/LivenessAnalysis.h"

std::pair<IR::FunctionCall*, IR::BasicBlock*>
Passes::RecursionToLoopPass::find_suitable_call(IR::Function& function) {
  // TODO: recursive call must be something like a bridge between start and some
  // ends

  if (!function.is_recursive()) {
    return {};
  }

  const auto& liveness =
      manager_.get_analysis<LivenessAnalysis>().get_liveness_info();

  IR::FunctionCall* recursive_call = nullptr;
  IR::BasicBlock* call_block = nullptr;

  for (auto& block : function.basic_blocks) {
    for (auto& instruction : block.instructions) {
      auto* call = dynamic_cast<IR::FunctionCall*>(instruction.get());

      if (call == nullptr || call->name != function.name) {
        continue;
      }

      // there must be only one recursive call in function
      if (recursive_call != nullptr) {
        return {};
      }

      recursive_call = call;
      call_block = &block;
    }
  }

  if (recursive_call == nullptr) {
    throw std::runtime_error(
        "Something wrong with function data. Function is recursive but doesn't "
        "contain recursive call.");
  }

  // after call only arguments and that call return value must be alive
  auto live_after_call = liveness.get_data(recursive_call, Position::AFTER);
  for (auto argument : function.arguments) {
    live_after_call.erase(argument);
  }
  live_after_call.erase(recursive_call->return_value);

  for (auto [value, is_live] : live_after_call) {
    if (is_live) {
      return {};
    }
  }

  return {recursive_call, call_block};
}

bool Passes::RecursionToLoopPass::apply(IR::Function& function) {
  auto [recursive_call, recursion_call_block] = find_suitable_call(function);

  if (recursive_call == nullptr) {
    return false;
  }

  // we must split block by recursive call
  auto call_itr = std::ranges::find_if(
      recursion_call_block->instructions,
      [recursive_call](const std::unique_ptr<IR::BaseInstruction>& ptr) {
        return ptr.get() == recursive_call;
      });

  function.split_block(*recursion_call_block, call_itr);

  // transforming recursion into loop
  auto after_recursion_block = recursion_call_block->children[0];
  recursion_call_block->children = {function.begin_block, nullptr};

  for (auto* end_block : function.end_blocks) {
    if (end_block != recursion_call_block) {
      end_block->children = {after_recursion_block, nullptr};
    }
  }

  // we add empty entry block for function (because phi instruction requires
  // parent block for each value)
  auto recursion_loop_header = function.begin_block;

  auto new_entry = function.add_block();
  new_entry->children = {function.begin_block, nullptr};
  function.begin_block = new_entry;

  // we must replace all function arguments with new values in loop and add phi
  // nodes for them
  std::unordered_map<IR::Value, IR::Value> arguments_mapping;

  for (size_t i = 0; i < function.arguments.size(); ++i) {
    if (recursive_call->arguments[i] != function.arguments[i]) {
      arguments_mapping[function.arguments[i]] = function.allocate_vreg();
    }
  }

  std::unordered_set<IR::BasicBlock*> recursion_loop_blocks;
  get_blocks_before_recursive_call(recursion_loop_header, recursion_call_block,
                                   recursion_loop_blocks);

  for (auto block : recursion_loop_blocks) {
    for (auto& instruction : block->instructions) {
      instruction->replace_values(arguments_mapping);
    }
  }

  for (size_t i = 0; i < function.arguments.size(); ++i) {
    if (arguments_mapping.contains(function.arguments[i])) {
      auto phi = std::make_unique<IR::Phi>();

      phi->parents.emplace_back(function.begin_block, function.arguments[i]);
      phi->parents.emplace_back(recursion_call_block,
                                recursive_call->arguments[i]);

      phi->return_value = arguments_mapping[function.arguments[i]];

      recursion_loop_header->instructions.emplace_front(std::move(phi));
    }
  }

  // also we add counter into loop
  auto counter_first_value = function.allocate_vreg();
  auto counter_second_value = function.allocate_vreg();

  recursion_loop_header->instructions.emplace_front(
      std::make_unique<IR::Addition>(counter_second_value, counter_first_value,
                                     IR::Value(1, IR::ValueType::CONSTANT)));

  auto phi = std::make_unique<IR::Phi>();

  phi->parents.emplace_back(function.begin_block,
                            IR::Value(0, IR::ValueType::CONSTANT));
  phi->parents.emplace_back(recursion_call_block, counter_second_value);
  phi->return_value = counter_first_value;

  recursion_loop_header->instructions.emplace_front(std::move(phi));

  // remove recursive call from function
  auto recursive_call_destination =
      recursion_call_block->instructions.back()->get_return_value();
  recursion_call_block->instructions.pop_back();

  // we make one value for recursive call return value
  auto new_block = function.add_block();
  new_block->children = {after_recursion_block, nullptr};

  auto return_value_phi = std::make_unique<IR::Phi>();

  auto return_value_phi_return_value = function.allocate_vreg();
  return_value_phi->return_value = return_value_phi_return_value;

  for (auto end_block : function.end_blocks) {
    if (end_block != recursion_call_block) {
      auto return_value =
          static_cast<const IR::Return&>(*end_block->instructions.back())
              .arguments[0];

      end_block->instructions.pop_back();
      return_value_phi->parents.emplace_back(end_block, return_value);

      end_block->children = {new_block, nullptr};
    }
  }

  new_block->instructions.push_front(std::move(return_value_phi));

  // now we repeat last instructions while counter != 0
  auto branch_block = function.add_block();
  new_block->children = {branch_block, nullptr};

  auto return_block = function.add_block();
  auto new_counter_first_value = function.allocate_vreg();
  auto new_counter_second_value = function.allocate_vreg();

  branch_block->children = {return_block, after_recursion_block};

  after_recursion_block->children = {branch_block, nullptr};

  auto iterations_counter_phi = std::make_unique<IR::Phi>();

  iterations_counter_phi->parents.emplace_back(new_block, counter_first_value);
  iterations_counter_phi->parents.emplace_back(after_recursion_block,
                                               new_counter_second_value);
  iterations_counter_phi->return_value = new_counter_first_value;

  branch_block->instructions.push_back(std::move(iterations_counter_phi));
  branch_block->instructions.push_back(
      std::make_unique<IR::Branch>(new_counter_first_value));

  // decrement counter
  after_recursion_block->instructions.push_front(
      std::make_unique<IR::Subtraction>(new_counter_second_value,
                                        new_counter_first_value,
                                        IR::Value(1, IR::ValueType::CONSTANT)));

  // adding return into return block
  IR::Value return_value = static_cast<const IR::Return&>(
                               *after_recursion_block->instructions.back())
                               .arguments[0];
  after_recursion_block->instructions.pop_back();

  // add phi node for resulting value
  auto another_phi = std::make_unique<IR::Phi>();

  another_phi->parents.emplace_back(new_block, return_value_phi_return_value);
  another_phi->parents.emplace_back(after_recursion_block, return_value);
  another_phi->return_value = function.allocate_vreg();

  return_block->instructions.push_back(
      std::make_unique<IR::Return>(another_phi->return_value));

  std::unordered_map<IR::Value, IR::Value> recursive_call_result_mapping;
  recursive_call_result_mapping.emplace(recursive_call_destination,
                                        another_phi->return_value);

  branch_block->instructions.emplace_front(std::move(another_phi));

  for (auto& block : function.basic_blocks) {
    for (auto& instruction : block.instructions) {
      instruction->replace_values(recursive_call_result_mapping);
    }
  }

  return true;
}

void Passes::RecursionToLoopPass::get_blocks_before_recursive_call(
    IR::BasicBlock* current, const IR::BasicBlock* end,
    std::unordered_set<IR::BasicBlock*>& visited) {
  if (current == nullptr) {
    return;
  }

  if (visited.contains(current)) {
    return;
  }

  visited.insert(current);

  if (current == end) {
    return;
  }

  for (auto child : current->children) {
    get_blocks_before_recursive_call(child, end, visited);
  }
}
