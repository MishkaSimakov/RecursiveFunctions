#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_function_declaration(FunctionDecl& node) {
  // build function type
  auto arguments_view =
    node.parameters |
    std::views::transform([](const std::unique_ptr<ParameterDecl>& node) {
      return node->type->value;
    });
  std::vector arguments(arguments_view.begin(), arguments_view.end());

  Type* return_type = node.return_type->value;

  Type* type = context_.types_storage.make_type<FunctionType>(
      std::move(arguments), return_type);

  auto [itr, was_emplaced] =
      current_scope_->symbols.emplace(node.name, SymbolInfo::make_terminal(node, type));

  if (!was_emplaced) {
    auto name = context_.get_string(node.name);
    scold_user(node, fmt::format("Redefinition of function {}", name));
  }

  // traverse function arguments and body
  node.subscope = start_nested_scope();

  for (auto& parameter : node.parameters) {
    traverse(*parameter);
  }

  // we skip CompoundStmt node and return type
  for (auto& stmt : node.body->statements) {
    traverse(*stmt);
  }

  end_nested_scope();
  return true;
}
}  // namespace Front
