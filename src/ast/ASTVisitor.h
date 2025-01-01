#pragma once

#include "ASTContext.h"
#include "ast/Nodes.h"

template <typename Child, bool IsConst = false>
class ASTVisitor {
 private:
  Child& child() { return static_cast<Child&>(*this); }
  const Child& child() const { return static_cast<const Child&>(*this); }

 protected:
  template <typename T>
  using wrap_const = std::conditional_t<IsConst, const T, T>;

  wrap_const<ASTContext>& context_;
 public:
  explicit ASTVisitor(wrap_const<ASTContext>& context) : context_(context) {}

  bool traverse() { return traverse(*context_.root); }

  bool traverse(wrap_const<ASTNode>& node) {
    if (!child().before_traverse(node)) {
      return false;
    }

    switch (node.get_kind()) {
#define NODE(kind, type, snake_case)                 \
  case ASTNode::Kind::kind:                          \
    if (!child().traverse_##snake_case(              \
            static_cast<wrap_const<type>&>(node))) { \
      return false;                                  \
    }                                                \
    break;

#include "ast/NodesList.h"

#undef NODE
    }

    if (!child().after_traverse(node)) {
      return false;
    }

    return true;
  }

  bool traverse_token_node(wrap_const<TokenNode>& node) {
    return child().visit_token_node(node);
  }
  bool traverse_int_type(wrap_const<IntType>& node) {
    return child().visit_int_type(node);
  }
  bool traverse_compound_statement(wrap_const<CompoundStmt>& node) {
    if (!child().visit_compound_statement(node)) {
      return false;
    }

    for (auto& stmt : node.statements) {
      if (!traverse(*stmt)) {
        return false;
      }
    }

    return true;
  }
  bool traverse_program_declaration(wrap_const<ProgramDecl>& node) {
    if (!child().visit_program_declaration(node)) {
      return false;
    }

    for (auto& import_decl : node.imports) {
      if (!traverse(*import_decl)) {
        return false;
      }
    }
    for (auto& declaration : node.declarations) {
      if (!traverse(*declaration)) {
        return false;
      }
    }

    return true;
  }
  bool traverse_parameter_declaration(wrap_const<ParameterDecl>& node) {
    if (!child().visit_parameter_declaration(node)) {
      return false;
    }

    return traverse(*node.type);
  }

  bool traverse_function_declaration(wrap_const<FunctionDecl>& node) {
    if (!child().visit_function_declaration(node)) {
      return false;
    }

    for (auto& parameter : node.parameters) {
      if (!traverse(*parameter)) {
        return false;
      }
    }

    if (!traverse(*node.return_type)) {
      return false;
    }

    return traverse(*node.body);
  }
  bool traverse_return_statement(wrap_const<ReturnStmt>& node) {
    if (!child().visit_return_statement(node)) {
      return false;
    }

    return traverse(*node.value);
  }
  bool traverse_integer_literal(wrap_const<IntegerLiteral>& node) {
    return child().visit_integer_literal(node);
  }
  bool traverse_string_literal(wrap_const<StringLiteral>& node) {
    return child().visit_string_literal(node);
  }
  bool traverse_id_expression(wrap_const<IdExpr>& node) {
    return child().visit_id_expression(node);
  }
  bool traverse_import_declaration(wrap_const<ImportDecl>& node) {
    return child().visit_import_declaration(node);
  }
  bool traverse_bool_type(wrap_const<BoolType>& node) {
    return child().visit_bool_type(node);
  }
  bool traverse_binary_operator(wrap_const<BinaryOperator>& node) {
    if (!child().visit_binary_operator(node)) {
      return false;
    }
    if (!traverse(*node.left)) {
      return false;
    }
    if (!traverse(*node.right)) {
      return false;
    }

    return true;
  }
  bool traverse_call_expression(wrap_const<CallExpr>& node) {
    if (!child().visit_call_expression(node)) {
      return false;
    }
    for (auto& argument : node.arguments) {
      if (!traverse(*argument)) {
        return false;
      }
    }

    return true;
  }

  bool before_traverse(wrap_const<ASTNode>& node) { return true; }
  bool after_traverse(wrap_const<ASTNode>& node) { return true; }

#define NODE(kind, type, snake_case) \
  bool visit_##snake_case(wrap_const<type>&) { return true; } \

#include "ast/NodesList.h"

#undef NODE
};
