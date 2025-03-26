#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <iostream>
#include <memory>
#include <vector>

#include "ast/ASTVisitor.h"
#include "compilation/ModuleContext.h"
#include "utils/Printing.h"

namespace Front {
struct ASTPrinterConfig : ASTVisitorConfig {
  static constexpr auto order() { return Order::PREORDER; }
  static constexpr auto is_const() { return true; }
  static constexpr auto override_all() { return false; }
};

class ASTPrinter : public ASTVisitor<ASTPrinter, ASTPrinterConfig>,
                   public TreePrinter {
  const ModuleContext& context_;

  std::string range_string(const ASTNode& node) {
    return fmt::format("<{}:{}>", node.source_range.begin.pos_id,
                       node.source_range.end.pos_id);
  }

 public:
  explicit ASTPrinter(const ModuleContext& module_context, std::ostream& os)
      : TreePrinter(os), context_(module_context) {}

  NodeTraverseType before_traverse(const ASTNode&) {
    move_cursor_down();
    return NodeTraverseType::CONTINUE;
  }
  bool after_traverse(const ASTNode&) {
    move_cursor_up();
    return true;
  }
  bool visit_type_node(const TypeNode& value) {
    add_node(fmt::format("Type {} {}", range_string(value),
                         value.value->to_string()));
    return true;
  }
  bool visit_compound_statement(const CompoundStmt& value) {
    add_node(fmt::format("CompoundStmt {}", range_string(value)));
    return true;
  }
  bool visit_program_declaration(const ProgramDecl& value) {
    add_node(fmt::format("ProgramDecl {}", range_string(value)));
    return true;
  }
  bool visit_function_declaration(const FunctionDecl& value) {
    std::vector<std::string> specifiers;
    if (value.specifiers.is_exported()) {
      specifiers.push_back("export");
    }
    if (value.specifiers.is_extern()) {
      specifiers.push_back("extern");
    }

    std::string_view name = context_.get_string(value.name);
    add_node(fmt::format("FuncDecl {} {} {}", range_string(value), name,
                         fmt::join(specifiers, " ")));
    return true;
  }
  bool visit_return_statement(const ReturnStmt& value) {
    add_node(fmt::format("ReturnStmt {}", range_string(value)));
    return true;
  }
  bool visit_integer_literal(const IntegerLiteral& value) {
    add_node(
        fmt::format("IntegerLiteral {} {}", range_string(value), value.value));
    return true;
  }
  bool visit_string_literal(const StringLiteral& value) {
    const auto& string = context_.get_string(value.id);
    add_node(fmt::format("StringLiteral {} {}", range_string(value), string));
    return true;
  }
  bool visit_bool_literal(const BoolLiteral& value) {
    add_node(
        fmt::format("BoolLiteral {} {}", range_string(value), value.value));
    return true;
  }
  bool visit_id_expression(const IdExpr& value) {
    auto qualified_name =
        value.id.parts | std::views::transform([this](StringId id) {
          return context_.get_string(id);
        });

    add_node(fmt::format("IdExpr {} {}", range_string(value),
                         fmt::join(qualified_name, "::")));
    return true;
  }
  bool visit_import_declaration(const ImportDecl& value) {
    const auto& string = context_.get_string(value.id);
    add_node(fmt::format("ImportDecl {} {}", range_string(value), string));
    return true;
  }
  bool visit_call_expression(const CallExpr& value) {
    add_node(fmt::format("CallExpr {}", range_string(value)));
    return true;
  }
  bool visit_binary_operator(const BinaryOperator& value) {
    auto op_string = value.get_string_representation();
    add_node(fmt::format("BinaryOp {} {}", range_string(value), op_string));
    return true;
  }
  bool visit_unary_operator(const UnaryOperator& value) {
    auto op_string = value.get_string_representation();
    add_node(fmt::format("UnaryOp {} {}", range_string(value), op_string));
    return true;
  }
  bool visit_variable_declaration(const VariableDecl& value) {
    std::string_view name = context_.get_string(value.name);
    add_node(fmt::format("VariableDecl {} {} {}", range_string(value), name,
                         value.type->value->to_string()));
    return true;
  }
  bool visit_assignment_statement(const AssignmentStmt& value) {
    add_node(fmt::format("AssignmentStmt {}", range_string(value)));
    return true;
  }
  bool visit_declaration_statement(const DeclarationStmt& value) {
    add_node(fmt::format("DeclarationStmt {}", range_string(value)));
    return true;
  }
  bool visit_expression_statement(const ExpressionStmt& value) {
    add_node(fmt::format("ExpressionStmt {}", range_string(value)));
    return true;
  }
  bool visit_namespace_declaration(const NamespaceDecl& value) {
    std::string_view name = context_.get_string(value.name);
    add_node(fmt::format("NamespaceDecl {} {}", range_string(value), name));
    return true;
  }
  bool visit_while_statement(const WhileStmt& value) {
    add_node(fmt::format("WhileStmt {}", range_string(value)));
    return true;
  }
  bool visit_if_statement(const IfStmt& value) {
    add_node(fmt::format("IfStmt {}", range_string(value)));
    return true;
  }
  bool visit_implicit_lvalue_to_rvalue_conversion_expression(
      const ImplicitLvalueToRvalueConversionExpr& value) {
    add_node(fmt::format("ImplicitLvalueToRvalueConversionExpr {}",
                         range_string(value)));
    return true;
  }

  void print() {
    traverse(*context_.ast_root);
    TreePrinter::print();
  }
};
}  // namespace Front
