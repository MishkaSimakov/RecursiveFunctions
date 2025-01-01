#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <iostream>
#include <memory>
#include <vector>

#include "ast/ASTVisitor.h"
#include "sources/SourceManager.h"

class ASTPrinter : public ASTVisitor<ASTPrinter, true> {
  struct TextNode {
    std::string value;
    std::vector<std::unique_ptr<TextNode>> children;

    TextNode(std::string value) : value(std::move(value)) {}
  };

  std::ostream& os_;
  const SourceManager& source_manager_;
  std::vector<TextNode*> nodes_stack_;

  std::string range_string(const ASTNode& node) {
    return fmt::format("<{}:{}>", node.source_begin.pos_id,
                       node.source_end.pos_id);
  }

  void add_node(std::string value) {
    nodes_stack_.pop_back();

    auto node = std::make_unique<TextNode>(std::move(value));
    auto node_ptr = node.get();
    nodes_stack_.back()->children.push_back(std::move(node));

    nodes_stack_.push_back(node_ptr);
  }

  void print_recursive(const TextNode& node,
                       std::vector<std::string>& prefixes) {
    for (auto& str : prefixes) {
      os_ << str;
    }

    os_ << node.value << "\n";

    if (node.children.empty()) {
      return;
    }

    if (!prefixes.empty()) {
      if (prefixes.back() == "|-") {
        prefixes.back() = "| ";
      } else if (prefixes.back() == "`-") {
        prefixes.back() = "  ";
      }
    }

    for (size_t i = 0; i < node.children.size(); ++i) {
      prefixes.push_back(i + 1 == node.children.size() ? "`-" : "|-");
      print_recursive(*node.children[i], prefixes);
      prefixes.pop_back();
    }
  }

 public:
  explicit ASTPrinter(const ASTContext& context, std::ostream& os,
                      const SourceManager& source_manager)
      : ASTVisitor(context), os_(os), source_manager_(source_manager) {}

  bool before_traverse(const ASTNode&) {
    // dummy node for convenience
    nodes_stack_.push_back(nullptr);
    return true;
  }
  bool after_traverse(const ASTNode&) {
    nodes_stack_.pop_back();

    return true;
  }
  bool visit_int_type(const IntType& value) {
    add_node(fmt::format("IntType {}", range_string(value)));
    return true;
  }
  bool visit_bool_type(const BoolType& value) {
    add_node(fmt::format("BoolType {}", range_string(value)));
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
    std::string_view name = context_.symbols[value.id];
    add_node(fmt::format("ParamDecl {} {}", range_string(value), name));
    return true;
  }
  bool visit_function_declaration(const FunctionDecl& value) {
    std::vector<std::string> specifiers;
    if (value.is_exported) {
      specifiers.push_back("export");
    }

    std::string_view name = context_.symbols[value.name_id];
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
    const auto& string = context_.string_literals_table[value.id].value;
    add_node(fmt::format("StringLiteral {} {}", range_string(value), string));
    return true;
  }
  bool visit_id_expression(const IdExpr& value) {
    std::string_view name = context_.symbols[value.id];
    add_node(fmt::format("IdExpr {} {}", range_string(value), name));
    return true;
  }
  bool visit_import_declaration(const ImportDecl& value) {
    const auto& string = context_.string_literals_table[value.id].value;
    add_node(fmt::format("ImportDecl {} {}", range_string(value), string));
    return true;
  }
  bool visit_call_expression(const CallExpr& value) {
    std::string_view name = context_.symbols[value.id];
    add_node(fmt::format("CallExpr {} {}", range_string(value), name));
    return true;
  }
  bool visit_binary_operator(const BinaryOperator& value) {
    char operator_name;
    switch (value.type) {
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

  void print() {
    auto root = std::make_unique<TextNode>("");
    nodes_stack_.push_back(root.get());

    traverse();

    std::vector<std::string> prefixes;
    print_recursive(*root->children.front(), prefixes);
    nodes_stack_.pop_back();
  }
};
