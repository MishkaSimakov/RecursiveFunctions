#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_call_expression(CallExpr& node) {
  auto* type = dynamic_cast<FunctionType*>(node.callee->type);

  // check that type is callable
  if (type == nullptr) {
    scold_user(node, "Expression type is not callable.");
  }

  if (type->arguments.size() != node.arguments.size()) {
    scold_user(node,
               fmt::format("Arguments count mismatch: {} != {}",
                           type->arguments.size(), node.arguments.size()));
  }

  for (size_t i = 0; i < type->arguments.size(); ++i) {
    if (type->arguments[i] != node.arguments[i]->type) {
      scold_user(*node.arguments[i], "Argument type mismatch: {} != {}",
                 type->arguments[i], node.arguments[i]->type);
    }

    as_initializer(node.arguments[i]);
  }

  node.type = type->return_type;
  node.value_category = ValueCategory::RVALUE;

  return true;
}
}  // namespace Front
