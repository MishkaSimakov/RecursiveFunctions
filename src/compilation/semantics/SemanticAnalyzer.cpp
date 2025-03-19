#include "SemanticAnalyzer.h"

#include <utils/Constants.h>

#include <iostream>

namespace Front {
TypesStorage& SemanticAnalyzer::types() { return context_.types_storage; }

void SemanticAnalyzer::scold_user(const ASTNode& node, std::string message) {
  errors_.emplace_back(node.source_range, std::move(message));
  throw SemanticAnalyzerException(std::move(errors_));
}

std::pair<Scope*, SymbolInfo*> SemanticAnalyzer::name_lookup(
    Scope* base_scope, StringId id, bool should_ascend) const {
  Scope* current_scope = base_scope;
  // first we do local lookup
  while (current_scope != nullptr) {
    auto itr = current_scope->symbols.find(id);
    if (itr != current_scope->symbols.end()) {
      return {current_scope, &itr->second};
    }

    if (!should_ascend) {
      return {nullptr, nullptr};
    }

    current_scope = current_scope->parent;
  }

  // if hasn't found then start global lookup in imported modules
  return recursive_global_name_lookup(context_, id);
}

std::pair<Scope*, SymbolInfo*> SemanticAnalyzer::qualified_name_lookup(
    Scope* base_scope, const IdExpr& qualified_id) {
  auto parts = qualified_id.parts;
  // process scope qualifiers
  for (size_t i = 0; i + 1 < parts.size(); ++i) {
    auto [next_scope, next_symbol] = name_lookup(base_scope, parts[i], i == 0);

    // only namespaces can appear as scope qualifiers
    if (!std::holds_alternative<NamespaceSymbol>(next_symbol->data)) {
      scold_user(qualified_id,
                 "Only namespace name can appear as qualifier before id.");
    }

    base_scope = std::get<NamespaceSymbol>(next_symbol->data).subscope;
  }

  return name_lookup(base_scope, parts.back(), false);
}

std::pair<Scope*, SymbolInfo*> SemanticAnalyzer::recursive_global_name_lookup(
    const ModuleContext& module, StringId name) const {
  // // TODO: notify when there are conflicting names
  // for (const auto& dependency : module.dependencies) {
  //   const auto& symbols = import_module.root_scope->symbols;
  //   auto itr = symbols.find(name);
  //   if (itr != symbols.end() && itr->second.is_exported) {
  //     return {itr->second.type, import_module.root_scope};
  //   }
  //
  //   auto recursive_search_result =
  //       recursive_global_name_lookup(import_module, name);
  //   if (recursive_search_result.first != nullptr) {
  //     return recursive_search_result;
  //   }
  // }
  //
  return {nullptr, nullptr};
}

void SemanticAnalyzer::check_call_arguments(FunctionType* type,
                                            const CallExpr& call) {
  if (type->arguments.size() != call.arguments.size()) {
    scold_user(call,
               fmt::format("Arguments count mismatch: {} != {}",
                           type->arguments.size(), call.arguments.size()));
  }

  for (size_t i = 0; i < type->arguments.size(); ++i) {
    if (type->arguments[i] != call.arguments[i]->type) {
      scold_user(*call.arguments[i],
                 fmt::format("Argument type mismatch: {} != {}",
                             type->arguments[i]->to_string(),
                             call.arguments[i]->type->to_string()));
    }
  }
}

bool SemanticAnalyzer::after_traverse(ASTNode& node) {
  if constexpr (Constants::debug) {
    Expression* expression = dynamic_cast<Expression*>(&node);
    if (expression != nullptr && expression->type == nullptr) {
      scold_user(
          node,
          "Program logic error. Type of expression is not calculated after "
          "traversal.");
    }
  }

  return true;
}

//
// bool SemanticAnalyzer::visit_function_declaration(FunctionDecl& node) {
// auto arguments_view =
//     node.parameters |
//     std::views::transform([](const std::unique_ptr<ParameterDecl>& node) {
//       return node->type->value;
//     });
// std::vector arguments(arguments_view.begin(), arguments_view.end());
//
// Type* return_type = node.return_type->value;
//
// Type* type = context_.types_storage.make_type<FunctionType>(
//     std::move(arguments), return_type);
// node.scope->add_symbol(node.name, type, node.is_exported);

//   return true;
// }
//
// bool SemanticAnalyzer::visit_variable_declaration(VariableDecl& node) {
//   Type* type = node.type->value;
//
//   if (node.initializer != nullptr && node.initializer->type != type) {
//     scold_user(
//         node.initializer->source_range,
//         fmt::format("Incompatible initializer and variable types: {} != {}",
//                     type->to_string(), node.initializer->type->to_string()));
//   }
//
//   node.scope->add_symbol(node.name, type, false);
//   return true;
// }
// bool SemanticAnalyzer::visit_parameter_declaration(ParameterDecl& node) {
//   Type* type = node.type->value;
//   node.scope->add_symbol(node.id, type, false);
//   return true;
// }
//
// bool SemanticAnalyzer::visit_integer_literal(IntegerLiteral& node) {
//   node.type = types().add_primitive<IntType>();
//   return true;
// }
//
// bool SemanticAnalyzer::visit_string_literal(StringLiteral& node) {
//   auto char_type = types().add_primitive<CharType>();
//   node.type = types().add_pointer(char_type);
//   return true;
// }
//
// bool SemanticAnalyzer::visit_binary_operator(BinaryOperator& node) {
//   // for now only "int op int" is allowed
//   if (*node.left->type != IntType()) {
//     scold_user(
//         node.left->source_range,
//         fmt::format(
//             "Incompatible type for binary operator. Must be int, got: {}",
//             node.left->type->to_string()));
//   }
//   if (*node.right->type != IntType()) {
//     scold_user(
//         node.right->source_range,
//         fmt::format(
//             "Incompatible type for binary operator. Must be int, got: {}",
//             node.right->type->to_string()));
//   }
//
//   node.type = types().add_primitive<IntType>();
//
//   return true;
// }
//
// bool SemanticAnalyzer::visit_return_statement(ReturnStmt& node) {
//   // must check that function type and function return are same
//   return true;
// }
}  // namespace Front
