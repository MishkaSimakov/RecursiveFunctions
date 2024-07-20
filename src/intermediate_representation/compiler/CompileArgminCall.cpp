#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const ArgminCallNode& node) {
  // asterisk_temporary_ = get_next_temporary();
  //
  // // * = 0
  // auto move_instruction = std::make_unique<Move>();
  // move_instruction->result_destination = asterisk_temporary_;
  // move_instruction->source = TemporaryOrConstant::constant(0);
  // result_.instructions.push_back(std::move(move_instruction));
  //
  // // main loop body
  // auto loop_block = std::make_shared<BasicBlock>();
  //
  //
  // result_.children.first = loop_block;
}
}  // namespace IR
