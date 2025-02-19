#pragma once

#include "ast/Nodes.h"

namespace Front {
enum class Order { POSTORDER, PREORDER };

template <typename Child, bool IsConst = false,
          Order DFSOrder = Order::PREORDER>
class ASTVisitor {
 private:
  Child& child() { return static_cast<Child&>(*this); }
  const Child& child() const { return static_cast<const Child&>(*this); }

 protected:
  template <typename T>
  using wrap_const = std::conditional_t<IsConst, const T, T>;

  wrap_const<ASTNode>& root_;

 public:
  enum class NodeTraverseType { CONTINUE, STOP, SKIP_NODE };

  explicit ASTVisitor(wrap_const<ASTNode>& root) : root_(root) {}

  bool traverse() { return traverse(root_); }

  bool traverse(wrap_const<ASTNode>& node) {
    auto traverse_type = child().before_traverse(node);
    if (traverse_type != NodeTraverseType::CONTINUE) {
      return traverse_type == NodeTraverseType::SKIP_NODE;
    }

    switch (node.get_kind()) {
#define NODE(kind, type, snake_case)                                           \
  case ASTNode::Kind::kind:                                                    \
    if constexpr (DFSOrder == Order::PREORDER) {                               \
      if (!child().visit_##snake_case(static_cast<wrap_const<type>&>(node))) { \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
    if (!child().traverse_##snake_case(                                        \
            static_cast<wrap_const<type>&>(node))) {                           \
      return false;                                                            \
    }                                                                          \
    if constexpr (DFSOrder == Order::POSTORDER) {                              \
      if (!child().visit_##snake_case(static_cast<wrap_const<type>&>(node))) { \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
    break;

#include "ast/NodesList.h"

#undef NODE
    }

    if (!child().after_traverse(node)) {
      return false;
    }

    return true;
  }

  bool traverse_variable_declaration(wrap_const<VariableDecl>& node) {
    if (!traverse(*node.type)) {
      return false;
    }
    if (node.initializer != nullptr && !traverse(*node.initializer)) {
      return false;
    }

    return true;
  }
  bool traverse_declaration_statement(wrap_const<DeclarationStmt>& node) {
    if (!traverse(*node.value)) {
      return false;
    }
    return true;
  }
  bool traverse_expression_statement(wrap_const<ExpressionStmt>& node) {
    if (!traverse(*node.value)) {
      return false;
    }
    return true;
  }
  bool traverse_type_node(wrap_const<TypeNode>& node) { return true; }
  bool traverse_compound_statement(wrap_const<CompoundStmt>& node) {
    for (auto& stmt : node.statements) {
      if (!traverse(*stmt)) {
        return false;
      }
    }

    return true;
  }
  bool traverse_program_declaration(wrap_const<ProgramDecl>& node) {
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
    return traverse(*node.type);
  }

  bool traverse_function_declaration(wrap_const<FunctionDecl>& node) {
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
    return traverse(*node.value);
  }
  bool traverse_integer_literal(wrap_const<IntegerLiteral>& node) {
    return true;
  }
  bool traverse_string_literal(wrap_const<StringLiteral>& node) { return true; }
  bool traverse_bool_literal(wrap_const<BoolLiteral>& node) { return true; }
  bool traverse_id_expression(wrap_const<IdExpr>& node) { return true; }
  bool traverse_import_declaration(wrap_const<ImportDecl>& node) {
    return true;
  }
  bool traverse_binary_operator(wrap_const<BinaryOperator>& node) {
    if (!traverse(*node.left)) {
      return false;
    }
    if (!traverse(*node.right)) {
      return false;
    }

    return true;
  }
  bool traverse_call_expression(wrap_const<CallExpr>& node) {
    if (!traverse(*node.name)) {
      return false;
    }

    for (auto& argument : node.arguments) {
      if (!traverse(*argument)) {
        return false;
      }
    }

    return true;
  }
  bool traverse_namespace_declaration(wrap_const<NamespaceDecl>& node) {
    for (auto& declaration : node.body) {
      if (!traverse(*declaration)) {
        return false;
      }
    }

    return true;
  }
  bool traverse_while_statement(wrap_const<WhileStmt>& node) {
    if (!traverse(*node.condition)) {
      return false;
    }

    return traverse(*node.body);
  }
  bool traverse_if_statement(wrap_const<IfStmt>& node) {
    if (!traverse(*node.condition)) {
      return false;
    }
    if (!traverse(*node.true_branch)) {
      return false;
    }
    if (!traverse(*node.false_branch)) {
      return false;
    }

    return true;
  }
  bool traverse_continue_statement(wrap_const<ContinueStmt>& node) {
    return true;
  }
  bool traverse_break_statement(wrap_const<BreakStmt>& node) { return true; }

  NodeTraverseType before_traverse(wrap_const<ASTNode>& node) {
    return NodeTraverseType::CONTINUE;
  }
  bool after_traverse(wrap_const<ASTNode>& node) { return true; }

#define NODE(kind, type, snake_case) \
  bool visit_##snake_case(wrap_const<type>&) { return true; }

#include "ast/NodesList.h"

#undef NODE
};
}  // namespace Front
