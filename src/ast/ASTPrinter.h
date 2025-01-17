#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <iostream>
#include <memory>
#include <vector>

#include "ast/ASTVisitor.h"
#include "utils/Printing.h"

class ASTPrinter : public ASTVisitor<ASTPrinter, true>, public TreePrinter {
  const GlobalContext& global_context_;
  const ModuleContext& context_;

  std::string range_string(const ASTNode& node) {
    return fmt::format("<{}:{}>", node.source_range.begin.pos_id,
                       node.source_range.end.pos_id);
  }


 public:
  explicit ASTPrinter(const GlobalContext& global_context,
                      const ModuleContext& module_context, std::ostream& os)
      : ASTVisitor(*module_context.ast_root), TreePrinter(os),
        global_context_(global_context),
        context_(module_context) {}

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
  bool visit_parameter_declaration(const ParameterDecl& value) {
    std::string_view name = global_context_.get_string(value.id);
    add_node(fmt::format("ParamDecl {} {}", range_string(value), name));
    return true;
  }
  bool visit_function_declaration(const FunctionDecl& value) {
    std::vector<std::string> specifiers;
    if (value.is_exported) {
      specifiers.push_back("export");
    }

    std::string_view name = global_context_.get_string(value.name);
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
    const auto& string = global_context_.get_string(value.id);
    add_node(fmt::format("StringLiteral {} {}", range_string(value), string));
    return true;
  }
  bool visit_id_expression(const IdExpr& value) {
    std::string_view name = global_context_.get_string(value.id);
    add_node(fmt::format("IdExpr {} {}", range_string(value), name));
    return true;
  }
  bool visit_import_declaration(const ImportDecl& value) {
    const auto& string = global_context_.get_string(value.id);
    add_node(fmt::format("ImportDecl {} {}", range_string(value), string));
    return true;
  }
  bool visit_call_expression(const CallExpr& value) {
    std::string_view name = global_context_.get_string(value.id);
    add_node(fmt::format("CallExpr {} {}", range_string(value), name));
    return true;
  }
  bool visit_binary_operator(const BinaryOperator& value) {
    char operator_name;
    switch (value.op_type) {
      case BinaryOperator::OpType::PLUS:
        operator_name = '+';
        break;
      case BinaryOperator::OpType::MINUS:
        operator_name = '-';
        break;
      case BinaryOperator::OpType::MULTIPLY:
        operator_name = '*';
        break;
    }

    add_node(fmt::format("BinaryOp {} {}", range_string(value), operator_name));
    return true;
  }
  bool visit_variable_declaration(const VariableDecl& value) {
    std::string_view name = global_context_.get_string(value.name);
    add_node(fmt::format("VariableDecl {} {} {}", range_string(value), name,
                         value.type->value->to_string()));
    return true;
  }
  bool visit_declaration_statement(const DeclarationStmt& value) {
    add_node(fmt::format("DeclarationStmt {}", range_string(value)));
    return true;
  }

  void print() {
    traverse();
    TreePrinter::print();
  }
};
