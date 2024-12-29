#pragma once

#include <charconv>
#include <memory>

#include "ASTContext.h"
#include "Nodes.h"
#include "sources/SourceManager.h"

class ASTBuildContext {
  const SourceManager& source_manager_;
  ASTContext context_;

  template <typename T, typename... Args>
    requires std::is_base_of_v<ASTNode, T>
  static std::unique_ptr<T> make_node(SourceRange source_range,
                                      Args&&... args) {
    return std::make_unique<T>(source_range, std::forward<Args>(args)...);
  }

  template <typename U, typename V>
    requires std::is_base_of_v<V, U>
  static std::unique_ptr<U> cast_move(std::unique_ptr<V> ptr) {
    U* casted = dynamic_cast<U*>(ptr.release());
    if (casted == nullptr) {
      throw std::runtime_error("Wrong node cast in ASTBuildContext.");
    }
    return std::unique_ptr<U>(casted);
  }

  template <typename U, typename V>
    requires std::is_base_of_v<V, U>
  static const U& cast_view(const std::unique_ptr<V>& ptr) {
    return dynamic_cast<const U&>(*ptr);
  }

  size_t get_string_literal_id(std::string_view value, bool is_import) {
    auto& table = context_.string_literals_table;

    for (size_t i = 0; i < context_.string_literals_table.size(); ++i) {
      if (table[i].value == value && table[i].is_import == is_import) {
        return i;
      }
    }

    table.emplace_back(value, is_import);
    return table.size() - 1;
  }

 public:
  using NodePtr = std::unique_ptr<ASTNode>;

  explicit ASTBuildContext(const SourceManager& manager)
      : source_manager_(manager) {}

  ASTContext&& move_context(std::unique_ptr<ASTNode> root) {
    context_.root = cast_move<ProgramDecl>(std::move(root));
    return std::move(context_);
  }

  template <typename T>
  NodePtr construct(SourceRange source_range, std::span<NodePtr> nodes) {
    return make_node<T>(source_range);
  }

  template <size_t Index = 0>
  NodePtr pass(SourceRange source_range, std::span<NodePtr> nodes) {
    nodes[Index]->source_begin = source_range.begin;
    nodes[Index]->source_end = source_range.end;
    return std::move(nodes[Index]);
  }

  NodePtr string_literal(SourceRange source_range, std::span<NodePtr> nodes) {
    auto& token_node = cast_view<TokenNode>(nodes.front());
    std::string_view string =
        source_manager_.get_file_view(token_node.source_range());

    size_t literal_id = get_string_literal_id(string, false);
    return make_node<StringLiteral>(source_range, literal_id);
  }

  NodePtr integer_literal(SourceRange source_range, std::span<NodePtr> nodes) {
    auto& token_node = cast_view<TokenNode>(nodes.front());
    auto string = source_manager_.get_file_view(token_node.source_range());

    int result{};
    auto [_, ec] =
        std::from_chars(string.data(), string.data() + string.size(), result);

    if (ec == std::errc()) {
    } else if (ec == std::errc::invalid_argument) {
      throw std::runtime_error(
          "Something wrong with lexer: INTEGER_LITERAL is not a number.");
    } else if (ec == std::errc::result_out_of_range) {
      throw std::runtime_error("Too big a number for integer literal.");
    }

    return std::make_unique<IntegerLiteral>(source_range, result);
  }

  NodePtr id_expression(SourceRange source_range, std::span<NodePtr> nodes) {
    auto& token_node = cast_view<TokenNode>(nodes.front());
    std::string_view string =
        source_manager_.get_file_view(token_node.source_range());

    return make_node<IdExpr>(source_range, string);
  }

  NodePtr return_statement(SourceRange source_range, std::span<NodePtr> nodes) {
    return make_node<ReturnStmt>(source_range,
                                 cast_move<Expression>(std::move(nodes[1])));
  }

  NodePtr parameter_declaration(SourceRange source_range,
                                std::span<NodePtr> nodes) {
    auto decl_type = cast_move<Type>(std::move(nodes[2]));

    auto& token_node = cast_view<TokenNode>(nodes.front());
    auto view = source_manager_.get_file_view(token_node.source_range());

    return make_node<ParameterDecl>(source_range, view, std::move(decl_type));
  }

  NodePtr function_definition(SourceRange source_range,
                              std::span<NodePtr> nodes) {
    auto& token = cast_view<TokenNode>(nodes[0]);
    auto name_view = source_manager_.get_file_view(token.source_range());
    auto parameters = cast_move<ParametersListDecl>(std::move(nodes[2]));
    auto return_type = cast_move<Type>(std::move(nodes[3]));
    auto body = cast_move<CompoundStmt>(std::move(nodes[5]));

    return make_node<FunctionDecl>(source_range, name_view,
                                   std::move(parameters->parameters),
                                   std::move(return_type), std::move(body));
  }

  NodePtr module_import(SourceRange source_range, std::span<NodePtr> nodes) {
    auto& token_node = cast_view<TokenNode>(nodes[1]);
    std::string_view view =
        source_manager_.get_file_view(token_node.source_range());

    size_t literal_id = get_string_literal_id(view, true);
    return make_node<ImportDecl>(source_range, literal_id);
  }

  template <typename ListRootT, typename ListItemT>
    requires std::is_base_of_v<ASTNode, ListRootT> &&
             std::is_base_of_v<ASTNode, ListItemT>
  NodePtr sequence(SourceRange source_range, std::span<NodePtr> nodes) {
    std::unique_ptr root(nodes.size() == 1
                             ? make_node<ListRootT>(source_range)
                             : cast_move<ListRootT>(std::move(nodes.front())));

    root->source_end = source_range.end;
    root->add_item(cast_move<ListItemT>(std::move(nodes.back())));

    return std::move(root);
  }
};
