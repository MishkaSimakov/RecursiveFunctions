#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_function_declaration(FunctionDecl& node) {
  NestedScopeRAII scope_guard(current_scope_);

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

  if (current_scope_->has_symbol(node.name)) {
    auto name = context_.get_string(node.name);
    scold_user(node, fmt::format("Redefinition of function {:?}.", name));
  }

  SymbolInfo& info =
      current_scope_->parent->add_function(node.name, node, type);
  current_scope_->name = node.name;
  context_.functions_info.emplace(&node, info);

  for (auto& parameter : node.parameters) {
    traverse(*parameter);
  }

  if (!node.specifiers.is_extern()) {
    // we skip CompoundStmt node and return type
    for (auto& stmt : node.body->statements) {
      traverse(*stmt);
    }
  }

  return true;
}
}  // namespace Front
