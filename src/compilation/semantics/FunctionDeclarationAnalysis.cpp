#include "SemanticAnalyzer.h"

namespace Front {
Type* SemanticAnalyzer::add_to_transformations_if_necessary(
    const FunctionSymbolInfo& function) {
  // function is a transformation of type T
  // if it's first argument has type T where T is not a pointer.
  // If function f is a transformation of T, then expression t.f(args...) is
  // equivalent to f(t, args...), where t has type T

  FunctionType* type = function.type;
  if (type->get_arguments().size() == 0 ||
      type->get_arguments()[0]->get_kind() == Type::Kind::POINTER) {
    // not a transformation
    return nullptr;
  }

  Type* T = type->get_arguments()[0];

  if (T->get_kind() == Type::Kind::STRUCT) {
    for (auto name : static_cast<StructType*>(T)->get_members() | std::views::keys) {
      if (name == function.declaration.name) {
        scold_user(function.declaration,
                   "{:?} is a transformation of T={:?}, but it's name "
                   "conflicts with T's members names.",
                   function.declaration.name, T);
      }
    }
  }

  return T;
}

bool SemanticAnalyzer::traverse_function_declaration(FunctionDecl& node) {
  Scope& scope = *current_scope_;
  if (scope.has_symbol(node.name)) {
    auto name = context_.get_string(node.name);
    scold_user(node, fmt::format("name '{}' is already declared", name));
  }

  Scope& subscope = current_scope_->add_child(node.name);

  SymbolInfo& info = scope.add_function(node.name, node, nullptr, &subscope);
  FunctionSymbolInfo& fun_info = info.as<FunctionSymbolInfo>();

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
  fun_info.transformation_type = add_to_transformations_if_necessary(fun_info);

  context_.symbols_info.emplace(&node, info);

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
