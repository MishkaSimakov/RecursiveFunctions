#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ArgminOperatorNode& node) {
  FunctionCall argmin_call(Value{}, ArgminOperatorNode::operator_name);
  compiled_calls_stack_.push(std::move(argmin_call));

  // we allocate 2 temporaries for the sake of ssa
  auto other_asterisk_temporary = get_next_temporary();
  asterisk_temporary_ = get_next_temporary();

  // main loop body
  BasicBlock* previous = result_;

  BasicBlock* loop_block = current_function_->add_block();
  result_->children[0] = loop_block;
  result_ = loop_block;

  auto phi_node = std::make_unique<Phi>();
  phi_node->return_value = asterisk_temporary_;
  phi_node->parents.emplace_back(previous, Value(0, ValueType::CONSTANT));
  phi_node->parents.emplace_back(loop_block, other_asterisk_temporary);

  loop_block->instructions.push_back(std::move(phi_node));

  node.wrapped->accept(*this);

  // asterisk += 1
  auto insert_itr = std::prev(result_->instructions.end());
  result_->instructions.insert(
      insert_itr,
      std::make_unique<Addition>(other_asterisk_temporary, asterisk_temporary_,
                                 Value(1, ValueType::CONSTANT)));

  // returning block
  auto end_block = current_function_->add_block();
  result_->children = {end_block, result_};

  result_ = end_block;

  compiled_calls_stack_.pop();

  assign_or_pass_as_argument(asterisk_temporary_);
}
}  // namespace IR
