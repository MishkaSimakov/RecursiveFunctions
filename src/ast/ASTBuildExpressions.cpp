#include <assert.h>

#include <charconv>

#include "ASTBuildContext.h"

namespace Front {
int64_t ASTBuildContext::get_number_from_token(const TokenNode& token) {
  std::string_view string = module_source_.string_view(token.source_range);

  int64_t result{};
  auto [_, ec] =
      std::from_chars(string.data(), string.data() + string.size(), result);

  if (ec == std::errc()) {
  } else if (ec == std::errc::invalid_argument) {
    assert(false && "Lexer error. NUMBER token contains not a number");
  } else if (ec == std::errc::result_out_of_range) {
    errors.emplace_back(token.source_range,
                        "Value is too big to be represented by i64 type.");
    return 0;
  }

  return result;
}

NodePtr ASTBuildContext::string_literal(SourceRange source_range,
                                        std::span<NodePtr> nodes) {
  std::string_view string =
      module_source_.string_view(nodes.front()->source_range);

  // remove quotes
  string.remove_prefix(1);
  string.remove_suffix(1);

  auto literal_id = strings_.add_string(string);
  return make_node<StringLiteral>(source_range, literal_id);
}

NodePtr ASTBuildContext::integer_literal(SourceRange source_range,
                                         std::span<NodePtr> nodes) {
  const TokenNode& token = cast_view<TokenNode>(nodes.front());
  int64_t result = get_number_from_token(token);

  return std::make_unique<IntegerLiteral>(source_range, result);
}

NodePtr ASTBuildContext::bool_literal(SourceRange source_range,
                                      std::span<NodePtr> nodes) {
  auto token = cast_view<TokenNode>(nodes.front());
  bool value = token.token_type == Lexis::TokenType::KW_TRUE;
  return make_node<BoolLiteral>(source_range, value);
}

NodePtr ASTBuildContext::id_expression(SourceRange source_range,
                                       std::span<NodePtr> nodes) {
  QualifiedId id = get_qualified_id(std::move(nodes.front()));
  return make_node<IdExpr>(source_range, std::move(id));
}

NodePtr ASTBuildContext::call_expression(SourceRange source_range,
                                         std::span<NodePtr> nodes) {
  auto callee = cast_move<Expression>(std::move(nodes[0]));
  auto arguments_list = cast_move<NodesList<Expression>>(std::move(nodes[1]));

  return std::make_unique<CallExpr>(source_range, std::move(callee),
                                    std::move(arguments_list->nodes));
}

NodePtr ASTBuildContext::tuple_index_expression(SourceRange source_range,
                                                std::span<NodePtr> nodes) {
  auto left = cast_move<Expression>(std::move(nodes[0]));
  int64_t index = get_number_from_token(cast_view<TokenNode>(nodes[2]));

  return make_node<TupleIndexExpr>(source_range, std::move(left), index);
}

NodePtr ASTBuildContext::explicit_cast_expression(SourceRange source_range,
                                                  std::span<NodePtr> nodes) {
  auto child = cast_move<Expression>(std::move(nodes[0]));
  auto type = cast_move<TypeNode>(std::move(nodes[2]));

  return make_node<ExplicitCastExpr>(source_range, std::move(child),
                                     std::move(type));
}

NodePtr ASTBuildContext::member_expression(SourceRange source_range,
                                           std::span<NodePtr> nodes) {
  auto left = cast_move<Expression>(std::move(nodes[0]));
  QualifiedId member = get_qualified_id(std::move(nodes[2]));

  return make_node<MemberExpr>(source_range, std::move(left),
                               std::move(member));
}

NodePtr ASTBuildContext::tuple_expression(SourceRange source_range,
                                          std::span<NodePtr> nodes) {
  std::vector<std::unique_ptr<Expression>> elements;
  if (nodes.size() == 4) {
    elements.push_back(cast_move<Expression>(std::move(nodes[1])));
  } else if (nodes.size() == 5) {
    elements.push_back(cast_move<Expression>(std::move(nodes[1])));

    auto list = cast_move<NodesList<Expression>>(std::move(nodes[3]));
    for (auto& element : list->nodes) {
      elements.push_back(std::move(element));
    }
  }

  return make_node<TupleExpr>(source_range, std::move(elements));
}

}  // namespace Front
