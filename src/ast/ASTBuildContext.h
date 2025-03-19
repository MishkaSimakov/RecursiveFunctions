#pragma once

#include <charconv>
#include <memory>

#include "Nodes.h"
#include "compilation/GlobalContext.h"
#include "compilation/types/TypesStorage.h"
#include "sources/SourceManager.h"

namespace Front {
class ASTBuildContext {
  ModuleContext& context_;
  SourceView module_source_;

  TypesStorage& types() { return context_.types_storage; }
  ModuleContext& module() { return context_; }

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

  QualifiedId get_qualified_id(std::unique_ptr<ASTNode> node) const {
    QualifiedId result;

    auto id_parts = cast_move<NodesList<TokenNode>>(std::move(node));
    for (const auto& part : id_parts->nodes) {
      std::string_view part_string = get_token_string(*part);
      StringId part_string_id = context_.add_string(part_string);
      result.parts.push_back(part_string_id);
    }

    return result;
  }

  std::string_view get_token_string(const TokenNode& node) const {
    return module_source_.string_view(node.source_range);
  }

 public:
  using NodePtr = std::unique_ptr<ASTNode>;

  ASTBuildContext(ModuleContext& context, SourceView module_source)
      : context_(context), module_source_(module_source) {}

  template <typename T>
  NodePtr construct(SourceRange source_range, std::span<NodePtr> nodes) {
    return make_node<T>(source_range);
  }

  template <size_t Index = 0>
  NodePtr pass(SourceRange source_range, std::span<NodePtr> nodes) {
    nodes[Index]->source_range = source_range;
    return std::move(nodes[Index]);
  }

  NodePtr program_declaration(SourceRange source_range,
                              std::span<NodePtr> nodes) {
    auto program_node = std::make_unique<ProgramDecl>(source_range);
    bool has_imports = nodes.size() == 2;
    bool has_declarations = nodes.size() >= 1;

    if (has_imports) {
      auto import_list = cast_move<NodesList<ImportDecl>>(std::move(nodes[0]));
      program_node->imports = std::move(import_list->nodes);
    }

    if (has_declarations) {
      auto decl_list =
          cast_move<NodesList<Declaration>>(std::move(nodes.back()));
      program_node->declarations = std::move(decl_list->nodes);
    }

    return std::move(program_node);
  }

  NodePtr string_literal(SourceRange source_range, std::span<NodePtr> nodes) {
    const auto& token_node = cast_view<TokenNode>(nodes.front());
    std::string_view string = get_token_string(token_node);

    // remove quotes
    string.remove_prefix(1);
    string.remove_suffix(1);

    auto literal_id = context_.add_string(string);
    return make_node<StringLiteral>(source_range, literal_id);
  }

  NodePtr integer_literal(SourceRange source_range, std::span<NodePtr> nodes) {
    const auto& token_node = cast_view<TokenNode>(nodes.front());
    auto string = get_token_string(token_node);

    int result{};
    auto [_, ec] =
        std::from_chars(string.data(), string.data() + string.size(), result);

    if (ec == std::errc()) {
    } else if (ec == std::errc::invalid_argument) {
      throw std::runtime_error(
          "Something wrong with lexer: INTEGER_LITERAL is not a number.");
    } else if (ec == std::errc::result_out_of_range) {
      throw std::runtime_error(
          "Value is too big to be represented by int type.");
    }

    return std::make_unique<IntegerLiteral>(source_range, result);
  }

  NodePtr id_expression(SourceRange source_range, std::span<NodePtr> nodes) {
    QualifiedId id = get_qualified_id(std::move(nodes.front()));
    return make_node<IdExpr>(source_range, std::move(id));
  }

  NodePtr return_statement(SourceRange source_range, std::span<NodePtr> nodes) {
    std::unique_ptr<Expression> return_value =
        nodes.size() == 2 ? nullptr
                          : cast_move<Expression>(std::move(nodes[1]));

    return make_node<ReturnStmt>(source_range, std::move(return_value));
  }

  NodePtr parameter_declaration(SourceRange source_range,
                                std::span<NodePtr> nodes) {
    auto decl_type = cast_move<TypeNode>(std::move(nodes[2]));

    const auto& token_node = cast_view<TokenNode>(nodes.front());
    auto name_id = context_.add_string(get_token_string(token_node));

    return make_node<VariableDecl>(source_range, name_id, std::move(decl_type),
                                   nullptr);
  }

  NodePtr compound_statement(SourceRange source_range,
                             std::span<NodePtr> nodes) {
    std::unique_ptr<CompoundStmt> statements =
        nodes.size() == 3 ? cast_move<CompoundStmt>(std::move(nodes[1]))
                          : make_node<CompoundStmt>(source_range);

    statements->source_range = source_range;

    return std::move(statements);
  }

  NodePtr function_specifiers(SourceRange source_range,
                              std::span<NodePtr> nodes) {
    auto specifiers = std::make_unique<SpecifiersNode>(source_range);

    for (auto& node : nodes) {
      auto token = cast_move<TokenNode>(std::move(node));

      switch (token->token_type) {
        case Lexis::TokenType::KW_EXTERN:
          specifiers->set_extern(true);
          break;
        case Lexis::TokenType::KW_EXPORT:
          specifiers->set_exported(true);
          break;
        default:
          unreachable("Grammar must fail with other token types.");
      }
    }

    return std::move(specifiers);
  }

  template <bool IsDefined>
  NodePtr function(SourceRange source_range, std::span<NodePtr> nodes) {
    auto specifiers = cast_move<SpecifiersNode>(std::move(nodes[0]));

    const auto& id_token = cast_view<TokenNode>(nodes[1]);
    auto name_id = context_.add_string(get_token_string(id_token));

    auto parameters = cast_move<NodesList<VariableDecl>>(std::move(nodes[3]));
    auto return_type = cast_move<TypeNode>(std::move(nodes[4]));

    std::unique_ptr<CompoundStmt> body;
    if constexpr (IsDefined) {
      body = cast_move<CompoundStmt>(std::move(nodes[6]));
    } else {
      if (!specifiers->is_extern()) {
        throw std::runtime_error("Function without definition must be extern.");
      }
    }

    return make_node<FunctionDecl>(
        source_range, name_id, std::move(parameters->nodes),
        std::move(return_type), std::move(body), *specifiers);
  }

  NodePtr module_import(SourceRange source_range, std::span<NodePtr> nodes) {
    const auto& name_node = cast_view<StringLiteral>(nodes[1]);
    return make_node<ImportDecl>(source_range, name_node.id);
  }

  template <typename ListRootT, typename ListItemT>
    requires std::is_base_of_v<ASTNode, ListRootT> &&
             std::is_base_of_v<ASTNode, ListItemT>
  NodePtr sequence(SourceRange source_range, std::span<NodePtr> nodes) {
    std::unique_ptr root(nodes.size() == 1
                             ? make_node<ListRootT>(source_range)
                             : cast_move<ListRootT>(std::move(nodes.front())));

    root->source_range.end = source_range.end;
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

  template <UnaryOperator::OpType type>
  NodePtr unary_op(SourceRange source_range, std::span<NodePtr> nodes) {
    auto value = cast_move<Expression>(std::move(nodes.back()));

    return std::make_unique<UnaryOperator>(source_range, type,
                                           std::move(value));
  }

  NodePtr call_expression(SourceRange source_range, std::span<NodePtr> nodes) {
    auto callee = cast_move<Expression>(std::move(nodes[0]));
    auto arguments_list = cast_move<NodesList<Expression>>(std::move(nodes[1]));

    return std::make_unique<CallExpr>(source_range, std::move(callee),
                                      std::move(arguments_list->nodes));
  }

  NodePtr pointer_type(SourceRange source_range, std::span<NodePtr> nodes) {
    auto child = cast_move<TypeNode>(std::move(nodes[0]));

    child->value = types().add_pointer(child->value);
    child->source_range = source_range;

    return std::move(child);
  }

  template <typename T>
    requires std::is_base_of_v<Type, T>
  NodePtr construct_type(SourceRange source_range, std::span<NodePtr> nodes) {
    Type* type_ptr = types().add_primitive<T>();
    return make_node<TypeNode>(source_range, type_ptr);
  }

  NodePtr variable_declaration(SourceRange source_range,
                               std::span<NodePtr> nodes) {
    const TokenNode& name_node = cast_view<TokenNode>(nodes.front());
    StringId name_id = context_.add_string(get_token_string(name_node));

    auto type = cast_move<TypeNode>(std::move(nodes[2]));
    auto initializer = nodes.size() == 6
                           ? cast_move<Expression>(std::move(nodes[4]))
                           : nullptr;

    return make_node<VariableDecl>(source_range, name_id, std::move(type),
                                   std::move(initializer));
  }

  NodePtr assignment_statement(SourceRange source_range,
                               std::span<NodePtr> nodes) {
    auto left = cast_move<Expression>(std::move(nodes[0]));
    auto right = cast_move<Expression>(std::move(nodes[2]));

    return make_node<AssignmentStmt>(source_range, std::move(left),
                                     std::move(right));
  }

  template <typename T, typename Wrapper, size_t Index>
    requires std::is_base_of_v<ASTNode, T> &&
             std::is_base_of_v<ASTNode, Wrapper>
  NodePtr wrap_pass(SourceRange source_range, std::span<NodePtr> nodes) {
    auto wrappee = cast_move<T>(std::move(nodes[Index]));
    return make_node<Wrapper>(source_range, std::move(wrappee));
  }

  NodePtr namespace_definition(SourceRange source_range,
                               std::span<NodePtr> nodes) {
    auto specifiers = cast_move<SpecifiersNode>(std::move(nodes[0]));

    if (specifiers->is_extern()) {
      throw std::runtime_error(
          "Extern specifier is not applicable to namespace.");
    }

    const auto& id_token = cast_view<TokenNode>(nodes[1]);
    auto name_id = context_.add_string(get_token_string(id_token));

    auto body = cast_move<NodesList<Declaration>>(std::move(nodes[5]));

    return make_node<NamespaceDecl>(source_range, name_id,
                                    std::move(body->nodes),
                                    specifiers->is_exported());
  }

  NodePtr while_statement(SourceRange source_range, std::span<NodePtr> nodes) {
    auto condition = cast_move<Expression>(std::move(nodes[2]));
    auto body = cast_move<CompoundStmt>(std::move(nodes[4]));

    return make_node<WhileStmt>(source_range, std::move(condition),
                                std::move(body));
  }

  NodePtr if_statement(SourceRange source_range, std::span<NodePtr> nodes) {
    bool has_else = nodes.size() == 7;
    auto condition = cast_move<Expression>(std::move(nodes[2]));
    auto true_branch = cast_move<CompoundStmt>(std::move(nodes[4]));

    auto false_branch =
        has_else ? cast_move<CompoundStmt>(std::move(nodes[6]))
                 : make_node<CompoundStmt>(
                       SourceRange::empty_at(true_branch->source_range.end));

    return make_node<IfStmt>(source_range, std::move(condition),
                             std::move(true_branch), std::move(false_branch));
  }

  NodePtr bool_literal(SourceRange source_range, std::span<NodePtr> nodes) {
    auto token = cast_view<TokenNode>(nodes.front());
    bool value = token.token_type == Lexis::TokenType::KW_TRUE;
    return make_node<BoolLiteral>(source_range, value);
  }
};
}  // namespace Front
