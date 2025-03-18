#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_call_expression(CallExpr& node) {
  FunctionType* type = dynamic_cast<FunctionType*>(node.callee->type);

  // check that type is callable
  if (type == nullptr) {
    scold_user(node, "Expression type is not callable.");
  }

  // throws in case of error
  if (type->arguments.size() != node.arguments.size()) {
    scold_user(node,
               fmt::format("Arguments count mismatch: {} != {}",
                           type->arguments.size(), node.arguments.size()));
  }

  for (size_t i = 0; i < type->arguments.size(); ++i) {
    if (type->arguments[i] != node.arguments[i]->type) {
      scold_user(*node.arguments[i],
                 fmt::format("Argument type mismatch: {} != {}",
                             type->arguments[i]->to_string(),
                             node.arguments[i]->type->to_string()));
    }

    convert_to_rvalue(node.arguments[i]);
  }

  node.type = type->return_type;
  node.value_category = ValueCategory::RVALUE;

  return true;
}
}  // namespace Front
