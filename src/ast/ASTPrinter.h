#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <memory>
#include <vector>

#include "ast/ASTVisitor.h"
#include "compilation/ModuleContext.h"
#include "utils/Printing.h"

namespace Front {
struct ASTPrinterConfig : ASTVisitorConfig {
  static constexpr auto order() { return Order::PREORDER; }
  static constexpr auto is_const() { return true; }
  static constexpr auto override_all() { return true; }
};

class ASTPrinter : public ASTVisitor<ASTPrinter, ASTPrinterConfig>,
                   public TreePrinter {
  const ModuleContext& context_;

  const StringPool& strings() const { return context_.get_strings_pool(); }

  std::string range_string(const ASTNode& node) {
    return fmt::format("<{}:{}>", node.source_range.begin.pos_id,
                       node.source_range.end.pos_id);
  }

  std::string get_specifiers_string(const Declaration& decl) {
    std::vector<std::string> specifiers;
    if (decl.specifiers.is_exported()) {
      specifiers.push_back("export");
    }
    if (decl.specifiers.is_extern()) {
      specifiers.push_back("extern");
    }
    return specifiers.empty() ? ""
                              : fmt::format("{}", fmt::join(specifiers, " "));
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

  // program
  bool visit_program(const ProgramNode& value) {
    add_node(fmt::format("Program {}", range_string(value)));
    return true;
  }

  // declarations
  bool visit_import_declaration(const ImportDecl& value) {
    const auto& string = context_.get_string(value.name);
    add_node(fmt::format("ImportDecl {} {} {}", range_string(value), string,
                         get_specifiers_string(value)));
    return true;
  }
  bool visit_namespace_declaration(const NamespaceDecl& value) {
    std::string_view name = context_.get_string(value.name);
    add_node(fmt::format("NamespaceDecl {} {} {}", range_string(value), name,
                         get_specifiers_string(value)));
    return true;
  }
  bool visit_variable_declaration(const VariableDecl& value) {
    std::string_view name = context_.get_string(value.name);
    add_node(fmt::format("VariableDecl {} {} {}", range_string(value), name,
                         get_specifiers_string(value)));
    return true;
  }
  bool visit_function_declaration(const FunctionDecl& value) {
    std::string_view name = context_.get_string(value.name);
    add_node(fmt::format("FunctionDecl {} {} {}", range_string(value), name,
                         get_specifiers_string(value)));
    return true;
  }
  bool visit_type_alias_declaration(const TypeAliasDecl& value) {
    std::string_view name = context_.get_string(value.name);
    add_node(fmt::format("TypeAliasDecl {} {} {}", range_string(value), name,
                         get_specifiers_string(value)));
    return true;
  }
  bool visit_class_declaration(const ClassDecl& value) {
    std::string_view name = context_.get_string(value.name);
    add_node(fmt::format("ClassDecl {} {} {}", range_string(value), name,
                         get_specifiers_string(value)));
    return true;
  }

  // 2. Statements
  bool visit_compound_statement(const CompoundStmt& value) {
    add_node(fmt::format("CompoundStmt {}", range_string(value)));
    return true;
  }
  bool visit_return_statement(const ReturnStmt& value) {
    add_node(fmt::format("ReturnStmt {}", range_string(value)));
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
  bool visit_while_statement(const WhileStmt& value) {
    add_node(fmt::format("WhileStmt {}", range_string(value)));
    return true;
  }
  bool visit_if_statement(const IfStmt& value) {
    add_node(fmt::format("IfStmt {}", range_string(value)));
    return true;
  }
  bool visit_continue_statement(const ContinueStmt& value) {
    add_node(fmt::format("ContinueStmt {}", range_string(value)));
    return true;
  }
  bool visit_break_statement(const BreakStmt& value) {
    add_node(fmt::format("BreakStmt {}", range_string(value)));
    return true;
  }

  // expressions
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
    add_node(fmt::format("IdExpr {} {}", range_string(value),
                         value.id.to_string(strings())));
    return true;
  }
  bool visit_binary_operator(const BinaryOperator& value) {
    auto op_string = value.get_string_representation();
    add_node(
        fmt::format("BinaryOperator {} {}", range_string(value), op_string));
    return true;
  }
  bool visit_unary_operator(const UnaryOperator& value) {
    auto op_string = value.get_string_representation();
    add_node(
        fmt::format("UnaryOperator {} {}", range_string(value), op_string));
    return true;
  }
  bool visit_call_expression(const CallExpr& value) {
    add_node(fmt::format("CallExpr {}", range_string(value)));
    return true;
  }
  bool visit_member_expression(const MemberExpr& value) {
    add_node(fmt::format("MemberExpr {} {}", range_string(value),
                         value.member.to_string(strings())));
    return true;
  }
  bool visit_tuple_expression(const TupleExpr& value) {
    add_node(fmt::format("TupleExpr {}", range_string(value)));
    return true;
  }
  bool visit_implicit_lvalue_to_rvalue_conversion_expression(
      const ImplicitLvalueToRvalueConversionExpr& value) {
    add_node(fmt::format("ImplicitLvalueToRvalueConversionExpr {}",
                         range_string(value)));
    return true;
  }
  bool visit_tuple_index_expression(const TupleIndexExpr& value) {
    add_node(
        fmt::format("TupleIndexExpr {} {}", range_string(value), value.index));
    return true;
  }

  // types
  bool visit_pointer_type(const PointerTypeNode& value) {
    add_node(fmt::format("PointerType {}", range_string(value)));
    return true;
  }
  bool visit_primitive_type(const PrimitiveTypeNode& value) {
    add_node(fmt::format("PrimitiveType {} {}{}", range_string(value),
                         value.kind.to_string(), value.width));
    return true;
  }
  bool visit_tuple_type(const TupleTypeNode& value) {
    add_node(fmt::format("TupleType {}", range_string(value)));
    return true;
  }
  bool visit_user_defined_type(const UserDefinedTypeNode& value) {
    add_node(fmt::format("UserDefinedType {} {}", range_string(value),
                         value.name.to_string(strings())));
    return true;
  }

  void print() {
    traverse(*context_.ast_root);
    TreePrinter::print();
  }
};
}  // namespace Front
