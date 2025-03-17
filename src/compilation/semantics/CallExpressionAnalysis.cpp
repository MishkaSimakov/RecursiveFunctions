#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_call_expression(CallExpr& node) {
  FunctionType* type = dynamic_cast<FunctionType*>(node.callee->type);

  // check that type is callable
  if (type == nullptr) {
    scold_user(node, "Expression type is not callable.");
  }

  // throws in case of error
  check_call_arguments(type, node);
  node.type = type->return_type;

  return true;
}
}  // namespace Front
