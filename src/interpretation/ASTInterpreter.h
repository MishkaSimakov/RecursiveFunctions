#pragma once
#include <iostream>
#include <variant>

#include "ast/ASTVisitor.h"
#include "compilation/GlobalContext.h"
#include "compilation/ModuleContext.h"
#include "utils/TupleUtils.h"

namespace Interpretation {
using namespace Front;

class InterpreterException final : public std::runtime_error {
  SourceRange range_;

 public:
  InterpreterException(SourceRange range, std::string_view message)
      : std::runtime_error(message.data()), range_(range) {}

  InterpreterException(const ASTNode& node, std::string_view message)
      : InterpreterException(node.source_range, message) {}

  InterpreterException(const std::unique_ptr<ASTNode>& node,
                       std::string_view message)
      : InterpreterException(node->source_range, message) {}

  const SourceRange& get_range() const { return range_; }
};

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

struct ASTInterpreterConfig : ASTVisitorConfig {
  static constexpr auto order() { return Order::POSTORDER; }
  static constexpr auto is_const() { return true; }
  static constexpr auto override_all() { return false; }
};

class ASTInterpreter : public ASTVisitor<ASTInterpreter, ASTInterpreterConfig> {
  const ModuleContext& context_;
  size_t current_nesting_level_{0};
  std::unordered_map<StringId, ExpressionValue> variables_;
  std::unordered_map<const Expression*, ExpressionValue> values_;

  void assign_variable(const VariableDecl& variable, ExpressionValue value) {
    if (!is_compatible_with(value, *variable.type->value)) {
      throw InterpreterException(variable,
                                 "Incompatible types in variable assignment.");
    }

    variables_[variable.name] = value;
  }

  bool is_print_function(const Expression& callee) const {
    const auto* id_expr = dynamic_cast<const IdExpr*>(&callee);
    if (id_expr == nullptr) {
      return false;
    }

    // names can be qualified
    // we only allow unqualified print function call
    if (id_expr->id.parts.size() != 1) {
      return false;
    }

    if (context_.get_string(id_expr->id.parts.front()) != "print") {
      return false;
    }

    return true;
  }

 public:
  explicit ASTInterpreter(const ModuleContext& context) : context_(context) {}

  [[noreturn]] bool visit_namespace_declaration(const NamespaceDecl& value) {
    throw InterpreterException(value, "Namespaces are not implemented.");
  }

  [[noreturn]] bool visit_import_declaration(const ImportDecl& value) {
    throw InterpreterException(value, "Imports are not allowed.");
  }

  bool visit_function_declaration(const FunctionDecl& value);
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

  void interpret() { traverse(*context_.ast_root); }
};
}  // namespace Interpretation
