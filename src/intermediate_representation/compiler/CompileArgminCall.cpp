#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ArgminCallNode& node) {
  FunctionCall argmin_call;
  argmin_call.name = "argmin";
  compiled_calls_stack_.push(std::move(argmin_call));

  // we allocate 2 temporaries for the sake of ssa
  auto other_asterisk_temporary = get_next_temporary();
  asterisk_temporary_ = get_next_temporary();

  // main loop body

  BasicBlock* loop_block = result_;

  if (!result_->instructions.empty()) {
    loop_block = current_function_->add_block();

    result_->children.first = loop_block;
    loop_block->parents = {result_, loop_block};

    result_ = loop_block;
  }

  auto phi_node = std::make_unique<Phi>();
  phi_node->result_destination = asterisk_temporary_;
  phi_node->values = {TemporaryOrConstant::constant(0),
                      other_asterisk_temporary};

  loop_block->instructions.push_back(std::move(phi_node));

  node.wrapped_call->accept(*this);

  // asterisk += 1
  auto add_instruction = std::make_unique<Addition>();
  add_instruction->result_destination = other_asterisk_temporary;
  add_instruction->left = asterisk_temporary_;
  add_instruction->right = TemporaryOrConstant::constant(1);

  auto insert_itr = std::prev(result_->instructions.end());
  result_->instructions.insert(insert_itr, std::move(add_instruction));

  // returning block
  auto end_block = current_function_->add_block();
  result_->children = {end_block, result_};

  end_block->parents = {result_};

  result_ = end_block;

  compiled_calls_stack_.pop();

  assign_or_pass_as_argument(asterisk_temporary_);
}
}  // namespace IR
