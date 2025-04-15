#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_function_declaration(FunctionDecl& node) {
  Scope& scope = *current_scope_;
  if (scope.has_symbol(node.name)) {
    auto name = context_.get_string(node.name);
    scold_user(node, fmt::format("name '{}' is already declared", name));
  }

  Scope& subscope = current_scope_->add_child(node.name);

  SymbolInfo& info = scope.add_function(node.name, node, nullptr, &subscope);
  FunctionSymbolInfo& fun_info = std::get<FunctionSymbolInfo>(info);
  subscope.parent_symbol = &info;

  NestedScopeRAII scope_guard(*this, subscope);

  // traverse parameters to calculate their types
  for (auto& parameter : node.parameters) {
    traverse(*parameter);
  }

  traverse(*node.return_type);

  // build function type
  auto arguments_view =
      node.parameters |
      std::views::transform([](const std::unique_ptr<VariableDecl>& node) {
        return node->type->value;
      });
  std::vector arguments(arguments_view.begin(), arguments_view.end());

  Type* return_type = node.return_type->value;
  FunctionType* type =
      types().make_type<FunctionType>(std::move(arguments), return_type);
  fun_info.type = type;

  context_.functions_info.emplace(&node, fun_info);

  add_to_exported_if_necessary(info);

  if (!node.specifiers.is_extern()) {
    // we skip CompoundStmt node and return type
    for (auto& stmt : node.body->statements) {
      traverse(*stmt);
    }
  }

  return true;
}
}  // namespace Front
