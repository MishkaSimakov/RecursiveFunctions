#include "SemanticAnalyzer.h"

#include <utils/Constants.h>

#include <algorithm>
#include <iostream>

#include "ast/ASTPrinter.h"
#include "compilation/ScopePrinter.h"

namespace Front {
TypesStorage& SemanticAnalyzer::types() { return context_.types_storage; }

void SemanticAnalyzer::scold_user(const ASTNode& node, std::string message) {
  errors_.emplace_back(node.source_range, std::move(message));
  throw SemanticAnalyzerException(std::move(errors_));
}

StringId SemanticAnalyzer::import_external_string(
    StringId external_string, const StringPool& external_strings) {
  if (&external_strings == &context_.get_strings_pool()) {
    return external_string;
  }

  std::string_view string_view = external_strings.get_string(external_string);
  return context_.add_string(string_view);
}

QualifiedId SemanticAnalyzer::import_external_string(
    const QualifiedId& external_string, const StringPool& external_strings) {
  QualifiedId result;
  for (StringId part : external_string.parts) {
    result.parts.push_back(import_external_string(part, external_strings));
  }
  return result;
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

void SemanticAnalyzer::as_initializer(std::unique_ptr<Expression>& expression) {
  if (expression->value_category == ValueCategory::RVALUE) {
    return;
  }

  if (expression->type->get_original()->get_kind() == Type::Kind::TUPLE) {
    const Expression& tuple = *expression;
    expression = std::make_unique<ImplicitTupleCopyExpr>(tuple.source_range,
                                                         std::move(expression));
    expression->type = tuple.type;
    expression->value_category = ValueCategory::RVALUE;
  } else {
    convert_to_rvalue(expression);
  }
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
  SymbolInfo* info = current_scope_->get_parent_symbol();
  if (info == nullptr || !info->is_function()) {
    scold_user(node, "Return statements are allowed only inside function.");
  }

  FunctionType* fun_ty = std::get<FunctionSymbolInfo>(*info).type;

  if (node.value->type != fun_ty->return_type) {
    auto left_type = node.value->type->to_string(context_.get_strings_pool());
    auto right_type =
        fun_ty->return_type->to_string(context_.get_strings_pool());

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

void SemanticAnalyzer::add_to_exported_if_necessary(SymbolInfo& info) {
  Declaration& decl = info.get_declaration();

  if (decl.specifiers.is_exported()) {
    context_.exported_symbols.push_back(info);
  }
}

void SemanticAnalyzer::analyze() {
  OSO_FIRE();

  auto name = context_.add_string(fmt::format("module({})", context_.name));
  context_.root_scope = std::make_unique<Scope>(name);
  current_scope_ = context_.root_scope.get();

  for (ModuleContext& exported : context_.dependencies) {
    for (SymbolInfo& exported_symbol : exported.exported_symbols) {
      inject_symbol(exported, exported_symbol);
    }
  }

  traverse(*context_.ast_root);

  // ScopePrinter printer(context_.get_strings_pool(), *context_.root_scope,
  // std::cout);
  // printer.print();

  // ASTPrinter ast_printer(context_, std::cout);
  // ast_printer.print();
}
}  // namespace Front
