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
    scold_user(
        node,
        fmt::format("Function name conflicts with other declaration.", name));
  }

  SymbolInfo& info =
      current_scope_->parent->add_function(node.name, node, type);
  current_scope_->name = node.name;
  context_.functions_info.emplace(&node, info);

  bool prev_is_in_exported_scope = is_in_exported_scope_;
  if (node.specifiers.is_exported()) {
    if (is_in_exported_scope_) {
      scold_user(node,
                 "Symbol is marked \"exported\" but it is already in "
                 "exported context. Remove unnecessary \"exported\".");
    }

    // context_.exported_symbols.push_back(info);
    is_in_exported_scope_ = true;
  }

  for (auto& parameter : node.parameters) {
    traverse(*parameter);
  }

  if (!node.specifiers.is_extern()) {
    // we skip CompoundStmt node and return type
    for (auto& stmt : node.body->statements) {
      traverse(*stmt);
    }
  }

  is_in_exported_scope_ = prev_is_in_exported_scope;

  return true;
}
}  // namespace Front
