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

  // if it hasn't found then start global lookup in imported modules
  return recursive_global_name_lookup(context_, id);
}

std::pair<Scope*, SymbolInfo*> SemanticAnalyzer::qualified_name_lookup(
    Scope* base_scope, const IdExpr& qualified_id) {
  auto& parts = qualified_id.id.parts;
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

  bool has_qualifiers = parts.size() > 1;
  return name_lookup(base_scope, parts.back(), !has_qualifiers);
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
void SemanticAnalyzer::convert_to_rvalue(
    std::unique_ptr<Expression>& expression) {
  if (expression->value_category == ValueCategory::RVALUE) {
    return;
  }

  auto cast = std::make_unique<ImplicitLvalueToRvalueConversionExpr>(
      expression->source_range, std::move(expression));

  cast->type = cast->value->type;
  cast->value_category = ValueCategory::RVALUE;

  expression = std::move(cast);
}

bool SemanticAnalyzer::after_traverse(ASTNode& node) {
  if constexpr (Constants::debug) {
    Expression* expression = dynamic_cast<Expression*>(&node);
    if (expression != nullptr) {
      if (expression->type == nullptr) {
        scold_user(node, "Program logic error. Type is not calculated.");
      }

      if (expression->value_category == ValueCategory::UNKNOWN) {
        scold_user(node,
                   "Program logic error. Value category is not calculated.");
      }
    }
  }

  return true;
}

bool SemanticAnalyzer::visit_return_statement(ReturnStmt& node) {
  SymbolInfo* scope_info = current_scope_->get_scope_info();
  if (scope_info == nullptr || !scope_info->is_function()) {
    scold_user(node, "Return statements are allowed only in function scope.");
  }

  const auto& function = std::get<FunctionSymbol>(scope_info->data);

  if (node.value->type != function.type->return_type) {
    auto left_type = node.value->type->to_string();
    auto right_type = function.type->return_type->to_string();

    scold_user(
        node,
        fmt::format(
            "Expression type in return statement must be equal to function "
            "type. {:?} != {:?}.",
            left_type, right_type));
  }

  // right part of assignment must be rvalue
  // if it is not then lvalue-to-rvalue-conversion is applied
  convert_to_rvalue(node.value);

  return true;
}
}  // namespace Front
