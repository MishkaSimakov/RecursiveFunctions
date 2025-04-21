#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::is_transformation(CallExpr& node) {
  if (node.callee->get_kind() == ASTNode::Kind::MEMBER_EXPR) {
    auto& member_expr = node.callee->as<MemberExpr>();
    SymbolInfo& info = context_.members_info.at(&member_expr);

    if (info.is_function()) {
      return true;
    }
  }

  return false;
}

bool SemanticAnalyzer::visit_call_expression(CallExpr& node) {
  // check that type is callable
  if (node.callee->type->get_kind() != Type::Kind::FUNCTION) {
    scold_user(*node.callee, "callee type is not callable.");
  }

  std::vector<std::reference_wrapper<std::unique_ptr<Expression>>> arguments;
  if (is_transformation(node)) {
    arguments.push_back(node.callee->as<MemberExpr>().left);
  }
  for (auto& argument : node.arguments) {
    arguments.push_back(argument);
  }

  auto& type = node.callee->type->as<FunctionType>();

  if (type.arguments.size() != arguments.size()) {
    scold_user(*node.callee,
               fmt::format("arguments count mismatch: {} != {}",
                           type.arguments.size(), arguments.size()));
  }

  for (size_t i = 0; i < type.arguments.size(); ++i) {
    std::unique_ptr<Expression>& argument = arguments[i];
    Type* expected_type = type.arguments[i];

    if (expected_type != argument->type) {
      scold_user(*argument, "Argument type mismatch: {} != {}", expected_type,
                 argument->type);
    }

    as_initializer(argument);
  }

  node.type = type.return_type;
  node.value_category = ValueCategory::RVALUE;

  return true;
}
}  // namespace Front
