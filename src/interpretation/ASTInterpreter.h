#pragma once
#include <iostream>
#include <variant>

#include "ast/ASTVisitor.h"
#include "compilation/GlobalContext.h"
#include "compilation/ModuleContext.h"
#include "utils/TupleUtils.h"

namespace Interpretation {
using namespace Front;

using ExpressionValue = std::variant<int64_t, bool>;

inline bool is_compatible_with(ExpressionValue value, const Type& type) {
  return std::visit(
      Overloaded{
          [&type](int64_t) { return type.get_kind() == Type::Kind::INT; },
          [&type](bool) { return type.get_kind() == Type::Kind::BOOL; }},
      value);
}

inline void print_function_implementation(ExpressionValue value) {
  std::visit(
      [](auto value) { std::cout << std::boolalpha << value << std::endl; },
      value);
}

class ASTInterpreter
    : public ASTVisitor<ASTInterpreter, true, Order::POSTORDER> {
  const GlobalContext& global_context_;
  size_t current_nesting_level_{0};
  std::unordered_map<StringId, ExpressionValue> variables_;
  std::unordered_map<const Expression*, ExpressionValue> values_;

  void assign_variable(const VariableDecl& variable, ExpressionValue value) {
    if (!is_compatible_with(value, *variable.type->value)) {
      throw std::runtime_error("Incompatible types in variable assignment.");
    }

    variables_[variable.name] = value;
  }

  bool is_print_function(const IdExpr& name) const {
    // names can be qualified
    // we only allow unqualified print function call
    if (name.parts.size() != 1) {
      return false;
    }

    if (global_context_.get_string(name.parts.front()) != "print") {
      return false;
    }

    return true;
  }

 public:
  explicit ASTInterpreter(const GlobalContext& global_context,
                                 const ModuleContext& module_context)
      : ASTVisitor(*module_context.ast_root), global_context_(global_context) {}

  [[noreturn]] bool visit_namespace_declaration(const NamespaceDecl&) {
    throw std::runtime_error("Namespaces are not implemented.");
  }

  [[noreturn]] bool visit_import_declaration(const ImportDecl&) {
    throw std::runtime_error("Imports are not allowed.");
  }

  bool visit_function_declaration(const FunctionDecl& value) const;
  bool visit_variable_declaration(const VariableDecl& value);

  bool traverse_if_statement(const IfStmt& value);

  bool visit_integer_literal(const IntegerLiteral& value) {
    values_[&value] = static_cast<int64_t>(value.value);
    return true;
  }

  bool visit_bool_literal(const BoolLiteral& value) {
    values_[&value] = value.value;
    return true;
  }

  bool traverse_call_expression(const CallExpr& value);

  bool visit_id_expression(const IdExpr& value);

  bool visit_binary_operator(const BinaryOperator& value);
};
}  // namespace Interpretation
