#include "IRASTVisitor.h"

bool Front::IRASTVisitor::traverse_return_statement(const ReturnStmt& value) {
  traverse(*value.value);
  current_basic_block_->append_instruction<IR::Return>(result_location_);

  return true;
}
