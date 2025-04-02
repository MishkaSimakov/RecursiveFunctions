#pragma once

#include <memory>

#include "Nodes.h"
#include "sources/SourceManager.h"
#include "utils/StringPool.h"

namespace Front {
using NodePtr = std::unique_ptr<ASTNode>;
using NodeSpan = std::span<NodePtr>;

class ASTBuildContext {
  StringPool& strings_;
  SourceView module_source_;

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
      StringId part_string = get_node_string(*part);
      result.parts.push_back(part_string);
    }

    return result;
  }

  StringId get_node_string(const ASTNode& node) const {
    std::string_view string = module_source_.string_view(node.source_range);
    return strings_.add_string(string);
  }

  int64_t get_number_from_token(const TokenNode& token) const;

 public:
  ASTBuildContext(StringPool& strings, SourceView module_source)
      : strings_(strings), module_source_(module_source) {}

  NodePtr add_export_specifier(SourceRange source_range,
                               std::span<NodePtr> nodes) {
    auto decl = cast_move<Declaration>(std::move(nodes[1]));
    decl->specifiers.set_exported(true);
    return std::move(decl);
  }

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
    auto program_node = std::make_unique<ProgramNode>(source_range);
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

  // expressions
  NodePtr string_literal(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr integer_literal(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr bool_literal(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr id_expression(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr member_expression(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr tuple_expression(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr call_expression(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr tuple_index_expression(SourceRange source_range,
                                 std::span<NodePtr> nodes);

  // statements
  NodePtr return_stmt(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr compound_stmt(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr assignment_stmt(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr while_stmt(SourceRange source_range, std::span<NodePtr> nodes);
  NodePtr if_stmt(SourceRange source_range, std::span<NodePtr> nodes);

  NodePtr parameter_declaration(SourceRange source_range,
                                std::span<NodePtr> nodes);
  NodePtr variable_declaration(SourceRange source_range,
                               std::span<NodePtr> nodes);
  NodePtr namespace_definition(SourceRange source_range,
                               std::span<NodePtr> nodes);

  // functions
 private:
  NodePtr base_function_declaration(SourceRange source_range,
                                    std::span<NodePtr> nodes, bool is_external);

 public:
  NodePtr function_declaration(SourceRange source_range,
                               std::span<NodePtr> nodes) {
    return base_function_declaration(source_range, nodes, false);
  }

  NodePtr extern_function_declaration(SourceRange source_range,
                                      std::span<NodePtr> nodes) {
    nodes = nodes.subspan(1);
    return base_function_declaration(source_range, nodes, true);
  }

  NodePtr module_import(SourceRange source_range, std::span<NodePtr> nodes) {
    const auto& name_node = cast_view<StringLiteral>(nodes[1]);
    return make_node<ImportDecl>(source_range, name_node.id);
  }

  NodePtr type_alias(SourceRange source_range, std::span<NodePtr> nodes) {
    StringId alias = get_node_string(*nodes[0]);
    auto type = cast_move<TypeNode>(std::move(nodes[4]));

    return make_node<TypeAliasDecl>(source_range, alias, std::move(type));
  }
  NodePtr class_type(SourceRange source_range, std::span<NodePtr> nodes) {
    StringId name = get_node_string(*nodes[0]);
    auto body = cast_move<NodesList<Declaration>>(std::move(nodes[4]));
    return make_node<ClassDecl>(source_range, name, std::move(body->nodes));
  }

  template <typename ListRootT, typename ListItemT>
    requires std::is_base_of_v<ASTNode, ListRootT> &&
             std::is_base_of_v<ASTNode, ListItemT>
  NodePtr sequence(SourceRange source_range, std::span<NodePtr> nodes) {
    if (nodes.empty()) {
      return make_node<ListRootT>(source_range);
    }

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

  NodePtr pointer_type(SourceRange source_range, std::span<NodePtr> nodes) {
    auto child = cast_move<TypeNode>(std::move(nodes[0]));
    return make_node<PointerTypeNode>(source_range, std::move(child));
  }

  NodePtr tuple_type(SourceRange source_range, std::span<NodePtr> nodes) {
    std::vector<std::unique_ptr<TypeNode>> elements;
    if (nodes.size() == 4) {
      elements.push_back(cast_move<TypeNode>(std::move(nodes[1])));
    } else if (nodes.size() == 5) {
      elements.push_back(cast_move<TypeNode>(std::move(nodes[1])));

      auto list = cast_move<NodesList<TypeNode>>(std::move(nodes[3]));
      for (auto& element : list->nodes) {
        elements.push_back(std::move(element));
      }
    }

    return make_node<TupleTypeNode>(source_range, std::move(elements));
  }

  NodePtr user_defined_type(SourceRange source_range,
                            std::span<NodePtr> nodes) {
    auto qual_id = get_qualified_id(std::move(nodes[0]));
    return make_node<UserDefinedTypeNode>(source_range, std::move(qual_id));
  }

  template <Type::Kind kind, size_t width>
  NodePtr primitive_type(SourceRange source_range, std::span<NodePtr> nodes) {
    return make_node<PrimitiveTypeNode>(source_range, kind, width);
  }

  template <typename T, typename Wrapper, size_t Index>
    requires std::is_base_of_v<ASTNode, T> &&
             std::is_base_of_v<ASTNode, Wrapper>
  NodePtr wrap_pass(SourceRange source_range, std::span<NodePtr> nodes) {
    auto wrappee = cast_move<T>(std::move(nodes[Index]));
    return make_node<Wrapper>(source_range, std::move(wrappee));
  }
};
}  // namespace Front
