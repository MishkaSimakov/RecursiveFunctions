#pragma once

#include <fmt/format.h>

#include <iostream>

#include "ast/ASTVisitor.h"
#include "sources/SourceManager.h"

class ASTPrinter : public ASTVisitor<ASTPrinter, true> {
  std::ostream& os_;
  const SourceManager& source_manager_;

  size_t depth{0};

  void print_prefix() {
    if (depth == 0) {
      return;
    }

    for (size_t i = 0; i < depth - 1; ++i) {
      os_ << "|\t";
    }
    os_ << "|- ";
  }

  std::string range_string(const ASTNode& node) {
    return fmt::format("<{}:{}>", node.source_begin.pos_id,
                       node.source_end.pos_id);
  }

 public:
  explicit ASTPrinter(const ASTContext& context, std::ostream& os,
                      const SourceManager& source_manager)
      : ASTVisitor(context), os_(os), source_manager_(source_manager) {}

  bool before_traverse(const ASTNode&) {
    print_prefix();
    ++depth;
    return true;
  }

  bool after_traverse(const ASTNode&) {
    --depth;
    return true;
  }

  bool visit_token_node(const TokenNode& value) {
    os_ << fmt::format("TokenNode {} {}\n", range_string(value),
                       value.token_type.to_string());
    return true;
  }
  bool visit_int_type(const IntType& value) {
    os_ << fmt::format("IntType {}\n", range_string(value));
    return true;
  }
  bool visit_compound_statement(const CompoundStmt& value) {
    os_ << fmt::format("CompoundStmt {}\n", range_string(value));
    return true;
  }
  bool visit_program_declaration(const ProgramDecl& value) {
    os_ << fmt::format("ProgramDecl {}\n", range_string(value));
    return true;
  }
  bool visit_parameter_declaration(const ParameterDecl& value) {
    os_ << fmt::format("ParamDecl {} {}\n", range_string(value), value.id);
    return true;
  }
  bool visit_parameter_list_declaration(const ParametersListDecl& value) {
    os_ << fmt::format("ParamListDecl {}\n", range_string(value));
    return true;
  }
  bool visit_function_declaration(const FunctionDecl& value) {
    os_ << fmt::format("FuncDecl {} {}\n", range_string(value), value.name);
    return true;
  }
  bool visit_return_statement(const ReturnStmt& value) {
    os_ << fmt::format("ReturnStmt {}\n", range_string(value));
    return true;
  }
  bool visit_integer_literal(const IntegerLiteral& value) {
    os_ << fmt::format("IntegerLiteral {} {}\n", range_string(value),
                       value.value);
    return true;
  }
  bool visit_string_literal(const StringLiteral& value) {
    const auto& string = context_.string_literals_table[value.id].value;
    os_ << fmt::format("StringLiteral {} {}\n", range_string(value), string);
    return true;
  }
  bool visit_id_expression(const IdExpr& value) {
    os_ << fmt::format("IdExpr {} {}\n", range_string(value), value.name);
    return true;
  }
  bool visit_import_declaration(const ImportDecl& value) {
    const auto& string = context_.string_literals_table[value.id].value;
    os_ << fmt::format("ImportDecl {} {}\n", range_string(value), string);
    return true;
  }
};
