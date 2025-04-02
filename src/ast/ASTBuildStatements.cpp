#include "ASTBuildContext.h"

namespace Front {
NodePtr ASTBuildContext::return_stmt(SourceRange source_range,
                                     std::span<NodePtr> nodes) {
  std::unique_ptr<Expression> return_value =
      nodes.size() == 2 ? nullptr : cast_move<Expression>(std::move(nodes[1]));

  return make_node<ReturnStmt>(source_range, std::move(return_value));
}

NodePtr ASTBuildContext::compound_stmt(SourceRange source_range,
                                       std::span<NodePtr> nodes) {
  std::unique_ptr<CompoundStmt> statements =
      nodes.size() == 3 ? cast_move<CompoundStmt>(std::move(nodes[1]))
                        : make_node<CompoundStmt>(source_range);

  statements->source_range = source_range;

  return std::move(statements);
}

NodePtr ASTBuildContext::assignment_stmt(SourceRange source_range,
                                         std::span<NodePtr> nodes) {
  auto left = cast_move<Expression>(std::move(nodes[0]));
  auto right = cast_move<Expression>(std::move(nodes[2]));

  return make_node<AssignmentStmt>(source_range, std::move(left),
                                   std::move(right));
}

NodePtr ASTBuildContext::while_stmt(SourceRange source_range,
                                    std::span<NodePtr> nodes) {
  auto condition = cast_move<Expression>(std::move(nodes[2]));
  auto body = cast_move<CompoundStmt>(std::move(nodes[4]));

  return make_node<WhileStmt>(source_range, std::move(condition),
                              std::move(body));
}

NodePtr ASTBuildContext::if_stmt(SourceRange source_range,
                                 std::span<NodePtr> nodes) {
  bool has_else = nodes.size() == 7;
  auto condition = cast_move<Expression>(std::move(nodes[2]));
  auto true_branch = cast_move<CompoundStmt>(std::move(nodes[4]));

  auto false_branch = has_else ? cast_move<CompoundStmt>(std::move(nodes[6]))
                               : make_node<CompoundStmt>(SourceRange::empty_at(
                                     true_branch->source_range.end));

  return make_node<IfStmt>(source_range, std::move(condition),
                           std::move(true_branch), std::move(false_branch));
}

}  // namespace Front
