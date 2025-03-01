#include "TypesASTVisitor.h"

#include <iostream>

namespace Front {
TypesStorage& TypesASTVisitor::types() { return module_.types_storage; }

void TypesASTVisitor::scold_user(SourceRange range,
                                 const std::string& message) {
  context_.source_manager.add_annotation(range, message);
  context_.source_manager.print_annotations(std::cout);
  throw std::runtime_error(message);
}

std::pair<Type*, Scope*> TypesASTVisitor::name_lookup(Scope* base_scope,
                                                      StringId name) const {
  Scope* current_scope = base_scope;

  // first we do local lookup
  while (current_scope != nullptr) {
    auto itr = current_scope->symbols.find(name);
    if (itr != current_scope->symbols.end()) {
      return {itr->second.type, current_scope};
    }

    current_scope = current_scope->parent;
  }

  // if haven't found then start global lookup in imported modules
  return recursive_global_name_lookup(module_, name);
}

std::pair<Type*, Scope*> TypesASTVisitor::recursive_global_name_lookup(
    const ModuleContext& module, StringId name) const {
  // TODO: notify when there are conflicting names
  for (StringId import_id : module.imports) {
    const auto& import_module =
        *context_.module_names_mapping[context_.get_string(import_id)];
    const auto& symbols = import_module.root_scope->symbols;
    auto itr = symbols.find(name);
    if (itr != symbols.end() && itr->second.is_exported) {
      return {itr->second.type, import_module.root_scope};
    }

    auto recursive_search_result =
        recursive_global_name_lookup(import_module, name);
    if (recursive_search_result.first != nullptr) {
      return recursive_search_result;
    }
  }

  return {nullptr, nullptr};
}

void TypesASTVisitor::check_call_arguments(FunctionType* type,
                                           const CallExpr& call) {
  if (type->arguments.size() != call.arguments.size()) {
    scold_user(call.source_range,
               fmt::format("Arguments count mismatch: {} != {}",
                           type->arguments.size(), call.arguments.size()));
  }

  for (size_t i = 0; i < type->arguments.size(); ++i) {
    if (type->arguments[i] != call.arguments[i]->type) {
      scold_user(call.arguments[i]->source_range,
                 fmt::format("Argument type mismatch: {} != {}",
                             type->arguments[i]->to_string(),
                             call.arguments[i]->type->to_string()));
    }
  }
}

bool TypesASTVisitor::visit_function_declaration(FunctionDecl& node) {
  auto arguments_view =
      node.parameters |
      std::views::transform([](const std::unique_ptr<ParameterDecl>& node) {
        return node->type->value;
      });
  std::vector arguments(arguments_view.begin(), arguments_view.end());

  Type* return_type = node.return_type->value;

  Type* type = context_.types_storage.make_type<FunctionType>(
      std::move(arguments), return_type);
  node.scope->add_symbol(node.name, type, node.is_exported);

  return true;
}

bool TypesASTVisitor::visit_variable_declaration(VariableDecl& node) {
  Type* type = node.type->value;

  if (node.initializer != nullptr && node.initializer->type != type) {
    scold_user(
        node.initializer->source_range,
        fmt::format("Incompatible initializer and variable types: {} != {}",
                    type->to_string(), node.initializer->type->to_string()));
  }

  node.scope->add_symbol(node.name, type, false);
  return true;
}
bool TypesASTVisitor::visit_parameter_declaration(ParameterDecl& node) {
  Type* type = node.type->value;
  node.scope->add_symbol(node.id, type, false);
  return true;
}

bool TypesASTVisitor::visit_integer_literal(IntegerLiteral& node) {
  node.type = types().add_primitive<IntType>();
  return true;
}

bool TypesASTVisitor::visit_string_literal(StringLiteral& node) {
  auto char_type = types().add_primitive<CharType>();
  node.type = types().add_pointer(char_type);
  return true;
}

bool TypesASTVisitor::visit_id_expression(IdExpr& node) {
  auto [id_type, id_scope] = name_lookup(node.scope, node.parts.front());

  if (id_type == nullptr) {
    scold_user(node.source_range, "Unknown identifier");
  }

  node.type = id_type;
  node.name_scope = id_scope;

  return true;
}

bool TypesASTVisitor::visit_call_expression(CallExpr& node) {
  auto [type, scope] = name_lookup(node.scope, node.name->parts.front());
  if (type == nullptr) {
    scold_user(node.source_range, "Unknown identifier");
  }

  FunctionType* func_type = dynamic_cast<FunctionType*>(type);
  if (func_type == nullptr) {
    scold_user(node.source_range, "Identifier must be of function type");
  }

  // throws in case of error
  check_call_arguments(func_type, node);
  node.type = func_type->return_type;
  node.name->scope = scope;

  return true;
}

bool TypesASTVisitor::visit_binary_operator(BinaryOperator& node) {
  // for now only "int op int" is allowed
  if (*node.left->type != IntType()) {
    scold_user(
        node.left->source_range,
        fmt::format(
            "Incompatible type for binary operator. Must be int, got: {}",
            node.left->type->to_string()));
  }
  if (*node.right->type != IntType()) {
    scold_user(
        node.right->source_range,
        fmt::format(
            "Incompatible type for binary operator. Must be int, got: {}",
            node.right->type->to_string()));
  }

  node.type = types().add_primitive<IntType>();

  return true;
}

bool TypesASTVisitor::visit_return_statement(ReturnStmt& node) {
  // must check that function type and function return are same
  return true;
}
}  // namespace Front
