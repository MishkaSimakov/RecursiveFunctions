#pragma once

#include <charconv>
#include <memory>

#include "ASTContext.h"
#include "Nodes.h"
#include "compilation/types/TypesStorage.h"
#include "sources/SourceManager.h"

class ASTBuildContext {
  const SourceManager& source_manager_;
  ASTContext context_;
  std::unordered_map<std::string_view, size_t> symbols_mapping_;
  TypesStorage& types_storage_;

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

  size_t get_symbol_id(std::string_view value) {
    auto itr = symbols_mapping_.find(value);

    if (itr != symbols_mapping_.end()) {
      return itr->second;
    }

    size_t index = context_.symbols.size();
    context_.symbols.push_back(std::string(value));
    symbols_mapping_.emplace(value, index);
    return index;
  }

  size_t get_symbol_id(SourceRange source_range) {
    return get_symbol_id(source_manager_.get_file_view(source_range));
  }

 public:
  using NodePtr = std::unique_ptr<ASTNode>;

  explicit ASTBuildContext(const SourceManager& manager,
                           TypesStorage& types_storage)
      : source_manager_(manager), context_(), types_storage_(types_storage) {}

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

  NodePtr program_declaration(SourceRange source_range,
                              std::span<NodePtr> nodes) {
    auto program_node = std::make_unique<ProgramDecl>(source_range);
    if (nodes.size() == 2) {
      auto import_list = cast_move<NodesList<ImportDecl>>(std::move(nodes[0]));
      program_node->imports = std::move(import_list->nodes);
    }

    auto decl_list = cast_move<NodesList<Declaration>>(std::move(nodes.back()));
    program_node->declarations = std::move(decl_list->nodes);

    return std::move(program_node);
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
    size_t id =
        get_symbol_id(source_manager_.get_file_view(token_node.source_range()));

    return make_node<IdExpr>(source_range, id);
  }

  NodePtr return_statement(SourceRange source_range, std::span<NodePtr> nodes) {
    return make_node<ReturnStmt>(source_range,
                                 cast_move<Expression>(std::move(nodes[1])));
  }

  NodePtr parameter_declaration(SourceRange source_range,
                                std::span<NodePtr> nodes) {
    auto decl_type = cast_move<Type>(std::move(nodes[2]));

    auto& token_node = cast_view<TokenNode>(nodes.front());
    size_t name_id = get_symbol_id(token_node.source_range());

    return make_node<ParameterDecl>(source_range, name_id,
                                    std::move(decl_type));
  }

  NodePtr function_definition(SourceRange source_range,
                              std::span<NodePtr> nodes) {
    bool has_specifier = nodes.size() == 7;
    size_t shift = has_specifier ? 1 : 0;

    // specifiers
    auto& spec_token = cast_view<TokenNode>(nodes[0]);
    bool is_exported = has_specifier;

    // identifier
    auto& id_token = cast_view<TokenNode>(nodes[shift + 0]);
    size_t name_id = get_symbol_id(id_token.source_range());

    auto parameters =
        cast_move<NodesList<ParameterDecl>>(std::move(nodes[shift + 2]));
    auto return_type = cast_move<Type>(std::move(nodes[shift + 3]));
    auto body = cast_move<CompoundStmt>(std::move(nodes[shift + 5]));

    return make_node<FunctionDecl>(
        source_range, name_id, std::move(parameters->nodes),
        std::move(return_type), std::move(body), is_exported);
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

  template <BinaryOperator::OpType type>
  NodePtr binary_op(SourceRange source_range, std::span<NodePtr> nodes) {
    auto left = cast_move<Expression>(std::move(nodes.front()));
    auto right = cast_move<Expression>(std::move(nodes.back()));

    return std::make_unique<BinaryOperator>(source_range, type, std::move(left),
                                            std::move(right));
  }

  NodePtr call_expression(SourceRange source_range, std::span<NodePtr> nodes) {
    auto arguments_list = cast_move<NodesList<Expression>>(std::move(nodes[1]));
    auto id_token = cast_view<TokenNode>(nodes[0]);
    size_t id =
        get_symbol_id(source_manager_.get_file_view(id_token.source_range()));

    return std::make_unique<CallExpr>(source_range, id,
                                      std::move(arguments_list->nodes));
  }

  NodePtr pointer_type(SourceRange source_range, std::span<NodePtr> nodes) {
    auto child = cast_move<Type>(std::move(nodes[0]));
    Type* child_ptr = types_storage_.get_type_ptr(std::move(child), );
    return std::make_unique<PointerType>(source_range, child_ptr);
  }
};
