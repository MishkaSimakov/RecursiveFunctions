#pragma once

#include "ast/Nodes.h"
#include "utils/TupleUtils.h"

namespace Front {
enum class Order { POSTORDER, PREORDER };

struct ASTVisitorConfig {
  static constexpr auto order() { return Order::POSTORDER; }
  static constexpr auto is_const() { return false; }
  static constexpr auto override_all() { return false; }
};

template <typename T, int = (T(), 1)>
constexpr bool is_constexpr_helper(T) {
  return true;
}
constexpr bool is_constexpr_helper(...) { return false; }

template <auto T>
constexpr bool is_constexpr_v = is_constexpr_helper([] { T(); });

template <typename Child, typename Config = ASTVisitorConfig>
  requires(std::is_base_of_v<ASTVisitorConfig, Config> &&
           is_constexpr_v<Config::order> && is_constexpr_v<Config::is_const> &&
           is_constexpr_v<Config::override_all>)
class ASTVisitor {
 private:
  Child& child() { return static_cast<Child&>(*this); }
  const Child& child() const { return static_cast<const Child&>(*this); }

 protected:
  template <typename T>
  using wrap_const = std::conditional_t<Config::is_const(), const T, T>;

 public:
  enum class NodeTraverseType { CONTINUE, STOP, SKIP_NODE };

  ASTVisitor() = default;

  bool traverse(wrap_const<ASTNode>& node) {
    auto traverse_type = child().before_traverse(node);
    if (traverse_type != NodeTraverseType::CONTINUE) {
      return traverse_type == NodeTraverseType::SKIP_NODE;
    }

    switch (node.get_kind()) {
#define NODE(kind, type, snake_case)                                           \
  case ASTNode::Kind::kind:                                                    \
    if (!child().before_##snake_case(static_cast<wrap_const<type>&>(node))) {  \
      return false;                                                            \
    }                                                                          \
    if constexpr (Config::order() == Order::PREORDER) {                        \
      if (!child().visit_##snake_case(static_cast<wrap_const<type>&>(node))) { \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
    if (!child().traverse_##snake_case(                                        \
            static_cast<wrap_const<type>&>(node))) {                           \
      return false;                                                            \
    }                                                                          \
    if constexpr (Config::order() == Order::POSTORDER) {                       \
      if (!child().visit_##snake_case(static_cast<wrap_const<type>&>(node))) { \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
    if (!child().after_##snake_case(static_cast<wrap_const<type>&>(node))) {   \
      return false;                                                            \
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
  bool traverse_assignment_statement(wrap_const<AssignmentStmt>& node) {
    if (!traverse(*node.left)) {
      return false;
    }
    if (!traverse(*node.right)) {
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
  bool traverse_function_declaration(wrap_const<FunctionDecl>& node) {
    for (auto& parameter : node.parameters) {
      if (!traverse(*parameter)) {
        return false;
      }
    }

    if (!traverse(*node.return_type)) {
      return false;
    }

    if (node.body != nullptr && !traverse(*node.body)) {
      return false;
    }

    return true;
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
  bool traverse_unary_operator(wrap_const<UnaryOperator>& node) {
    return traverse(*node.value);
  }
  bool traverse_call_expression(wrap_const<CallExpr>& node) {
    if (!traverse(*node.callee)) {
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

  bool traverse_implicit_lvalue_to_rvalue_conversion_expression(
      wrap_const<ImplicitLvalueToRvalueConversionExpr>& node) {
    return traverse(*node.value);
  }

  NodeTraverseType before_traverse(wrap_const<ASTNode>& node) {
    return NodeTraverseType::CONTINUE;
  }
  bool after_traverse(wrap_const<ASTNode>& node) { return true; }

  // before_* and after_* are experimental
  // TODO: consider removing visit_*, leaving only before_* + after_*
#define NODE(kind, type, snake_case)                           \
  bool visit_##snake_case(wrap_const<type>&) { return true; }  \
  bool before_##snake_case(wrap_const<type>&) { return true; } \
  bool after_##snake_case(wrap_const<type>&) { return true; }

#include "ast/NodesList.h"

#undef NODE
};
}  // namespace Front
