#pragma once

#include <charconv>
#include <memory>

#include "Nodes.h"
#include "compilation/GlobalContext.h"
#include "compilation/types/TypesStorage.h"
#include "sources/SourceManager.h"

namespace Front {
class ASTBuildContext {
  size_t module_id;
  GlobalContext& context_;

  TypesStorage& types() { return context_.types_storage; }
  ModuleContext& module() { return context_.modules[module_id]; }

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

  std::string_view get_token_string(const TokenNode& node) const {
    return context_.source_manager.get_file_view(node.source_range);
  }

  Scope* make_scope() { return &module().scopes.emplace_back(); }
  void set_scope_recursively(ASTNode& node, Scope* scope);

 public:
  using NodePtr = std::unique_ptr<ASTNode>;

  explicit ASTBuildContext(size_t module_id, GlobalContext& context)
      : module_id(module_id), context_(context) {}

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
    if (nodes.size() == 2) {
      auto import_list = cast_move<NodesList<ImportDecl>>(std::move(nodes[0]));
      program_node->imports = std::move(import_list->nodes);

      auto imports_string_ids =
          program_node->imports |
          std::views::transform(
              [](const std::unique_ptr<ImportDecl>& node) { return node->id; });
      module().imports =
          std::vector(imports_string_ids.begin(), imports_string_ids.end());
    }

    auto decl_list = cast_move<NodesList<Declaration>>(std::move(nodes.back()));
    program_node->declarations = std::move(decl_list->nodes);

    Scope* root_scope = make_scope();
    set_scope_recursively(*program_node, root_scope);
    module().root_scope = root_scope;

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

  // TODO: maybe unite with sequence method
  NodePtr id_expression(SourceRange source_range, std::span<NodePtr> nodes) {
    auto& token_node = cast_view<TokenNode>(nodes.back());
    auto id = context_.add_string(get_token_string(token_node));

    std::unique_ptr root(nodes.size() == 1
                             ? make_node<IdExpr>(source_range)
                             : cast_move<IdExpr>(std::move(nodes.front())));

    root->source_range.end = source_range.end;
    root->add_qualifier(id);

    return std::move(root);
  }

  NodePtr return_statement(SourceRange source_range, std::span<NodePtr> nodes) {
    return make_node<ReturnStmt>(source_range,
                                 cast_move<Expression>(std::move(nodes[1])));
  }

  NodePtr parameter_declaration(SourceRange source_range,
                                std::span<NodePtr> nodes) {
    auto decl_type = cast_move<TypeNode>(std::move(nodes[2]));

    const auto& token_node = cast_view<TokenNode>(nodes.front());
    auto name_id = context_.add_string(get_token_string(token_node));

    return make_node<ParameterDecl>(source_range, name_id,
                                    std::move(decl_type));
  }

  NodePtr compound_statement(SourceRange source_range,
                             std::span<NodePtr> nodes) {
    std::unique_ptr<CompoundStmt> statements =
        nodes.size() == 3 ? cast_move<CompoundStmt>(std::move(nodes[1]))
                          : make_node<CompoundStmt>(source_range);

    statements->source_range = source_range;

    // create new scope
    Scope* scope = make_scope();
    set_scope_recursively(*statements, scope);

    return std::move(statements);
  }

  NodePtr function_definition(SourceRange source_range,
                              std::span<NodePtr> nodes) {
    bool has_specifier = nodes.size() == 7;
    size_t shift = has_specifier ? 1 : 0;

    // specifiers
    auto& spec_token = cast_view<TokenNode>(nodes[0]);
    bool is_exported = has_specifier;

    // identifier
    const auto& id_token = cast_view<TokenNode>(nodes[shift + 0]);
    auto name_id = context_.add_string(get_token_string(id_token));

    auto parameters =
        cast_move<NodesList<ParameterDecl>>(std::move(nodes[shift + 2]));
    auto return_type = cast_move<TypeNode>(std::move(nodes[shift + 3]));
    auto body = cast_move<CompoundStmt>(std::move(nodes[shift + 5]));

    // arguments lie in the same scope as function body
    for (auto& param : parameters->nodes) {
      set_scope_recursively(*param, body->scope);
    }

    return make_node<FunctionDecl>(
        source_range, name_id, std::move(parameters->nodes),
        std::move(return_type), std::move(body), is_exported);
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

  NodePtr call_expression(SourceRange source_range, std::span<NodePtr> nodes) {
    auto id_expr = cast_move<IdExpr>(std::move(nodes[0]));
    auto arguments_list = cast_move<NodesList<Expression>>(std::move(nodes[1]));

    return std::make_unique<CallExpr>(source_range, std::move(id_expr),
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

  template <typename T, typename Wrapper, size_t Index>
    requires std::is_base_of_v<ASTNode, T> &&
             std::is_base_of_v<ASTNode, Wrapper>
  NodePtr wrap_pass(SourceRange source_range,
                                std::span<NodePtr> nodes) {
    auto wrappee = cast_move<T>(std::move(nodes[Index]));
    return make_node<Wrapper>(source_range, std::move(wrappee));
  }

  NodePtr namespace_definition(SourceRange source_range,
                               std::span<NodePtr> nodes) {
    bool is_exported = nodes.size() == 6;
    size_t shift = is_exported ? 1 : 0;

    // identifier
    const auto& id_token = cast_view<TokenNode>(nodes[shift + 0]);
    auto name_id = context_.add_string(get_token_string(id_token));

    auto body = cast_move<NodesList<Declaration>>(std::move(nodes[shift + 4]));

    return make_node<NamespaceDecl>(source_range, name_id,
                                    std::move(body->nodes), is_exported);
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

    // TODO: make empty ranges and put them here
    auto false_branch = has_else ? cast_move<CompoundStmt>(std::move(nodes[6]))
                                 : make_node<CompoundStmt>(source_range);

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
