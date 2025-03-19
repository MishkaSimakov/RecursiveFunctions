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

class ASTInterpreter
    : public ASTVisitor<ASTInterpreter, true, Order::POSTORDER> {
  const ModuleContext& context_;
  std::ostream& os_;

  size_t current_nesting_level_{0};
  std::unordered_map<StringId, std::pair<ExpressionValue, Type*>> variables_;
  std::unordered_map<const Expression*, ExpressionValue> values_;

  static bool is_compatible_with(ExpressionValue value, const Type& type) {
    return std::visit(
        Overloaded{
            [&type](int64_t) { return type.get_kind() == Type::Kind::INT; },
            [&type](bool) { return type.get_kind() == Type::Kind::BOOL; }},
        value);
  }

  void print_function_implementation(ExpressionValue value) const {
    std::visit(
        [this](auto value) { os_ << std::boolalpha << value << std::endl; },
        value);
  }

  void assign_variable(SourceRange assignment_range, StringId name,
                       ExpressionValue value) {
    if (!is_compatible_with(value, *variables_.at(name).second)) {
      throw InterpreterException(assignment_range,
                                 "Incompatible types in variable assignment.");
    }

    variables_[name].first = value;
  }

  bool is_print_function(const IdExpr& name) const {
    // names can be qualified
    // we only allow unqualified print function call
    if (name.parts.size() != 1) {
      return false;
    }

    if (context_.get_string(name.parts.front()) != "print") {
      return false;
    }

    return true;
  }

 public:
  explicit ASTInterpreter(const ModuleContext& context, std::ostream& os)
      : context_(context), os_(os) {}

  [[noreturn]] bool visit_namespace_declaration(const NamespaceDecl& value) {
    throw InterpreterException(value, "Namespaces are not implemented.");
  }

  [[noreturn]] bool visit_import_declaration(const ImportDecl& value) {
    throw InterpreterException(value, "Imports are not allowed.");
  }

  bool visit_function_declaration(const FunctionDecl& value);
  bool visit_variable_declaration(const VariableDecl& value);
  bool visit_assignment_statement(const AssignmentStmt& value);

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
