#include "ASTBuildContext.h"

namespace Front {

NodePtr ASTBuildContext::variable_declaration(SourceRange source_range,
                                              std::span<NodePtr> nodes) {
  StringId name = get_node_string(*nodes[0]);

  auto type = cast_move<TypeNode>(std::move(nodes[2]));
  auto initializer =
      nodes.size() == 5 ? cast_move<Expression>(std::move(nodes[4])) : nullptr;

  return make_node<VariableDecl>(source_range, name, std::move(type),
                                 std::move(initializer));
}

NodePtr ASTBuildContext::namespace_definition(SourceRange source_range,
                                              std::span<NodePtr> nodes) {
  auto name = get_node_string(*nodes[0]);
  auto body = cast_move<NodesList<Declaration>>(std::move(nodes[4]));

  return make_node<NamespaceDecl>(source_range, name, std::move(body->nodes));
}

NodePtr ASTBuildContext::base_function_declaration(SourceRange source_range,
                                                   std::span<NodePtr> nodes,
                                                   bool is_external) {
  StringId name = get_node_string(*nodes[0]);

  auto parameters = cast_move<NodesList<VariableDecl>>(std::move(nodes[2]));
  auto return_type = cast_move<TypeNode>(std::move(nodes[4]));

  auto body =
      !is_external ? cast_move<CompoundStmt>(std::move(nodes[6])) : nullptr;

  auto function =
      make_node<FunctionDecl>(source_range, name, std::move(parameters->nodes),
                              std::move(return_type), std::move(body));
  function->specifiers.set_extern(is_external);

  return std::move(function);
}

NodePtr ASTBuildContext::parameter_declaration(SourceRange source_range,
                                               std::span<NodePtr> nodes) {
  auto decl_type = cast_move<TypeNode>(std::move(nodes[2]));
  auto name_id = get_node_string(*nodes[0]);

  return make_node<VariableDecl>(source_range, name_id, std::move(decl_type),
                                 nullptr);
}

}  // namespace Front
