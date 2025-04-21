#include "Constants.h"
#include "SemanticAnalyzer.h"
#include "ast/ASTConstructor.h"

namespace Front {

bool SemanticAnalyzer::traverse_class_declaration(ClassDecl& node) {
  Scope* subscope = &current_scope_->add_child(node.name);
  auto info = StructSymbolInfo(current_scope_, subscope, node);

  auto qualified_name = info.get_fully_qualified_name();
  info.type = types().make_type<StructType>(std::move(qualified_name));

  auto [itr, was_emplaced] = current_scope_->symbols.emplace(node.name, info);

  subscope->parent_symbol = &itr->second;
  if (node.specifiers.is_exported()) {
    context_.exported_symbols.push_back(itr->second);
  }

  NestedScopeRAII scope_guard(*this, *subscope);

  for (auto& decl : node.body) {
    traverse(*decl);
  }

  // add implicit `make` method
  auto constructor_decl = make_implicit_class_constructor_decl(info);

  traverse(*node.body.emplace_back(std::move(constructor_decl)));

  context_.structs_info.emplace(info.type, itr->second);

  return true;
}

std::unique_ptr<Declaration>
SemanticAnalyzer::make_implicit_class_constructor_decl(StructSymbolInfo& info) {
  StringId constructor_name = context_.add_string(Constants::constructor_name);

  StructType* type = info.type;
  auto source_range = SourceRange::empty_at(info.declaration.source_begin());

  std::vector<std::unique_ptr<VariableDecl>> arguments;
  for (auto [member_name, member_type] : type->members) {
    auto type_node =
        ASTConstructor::create_type_node(member_type, source_range);
    arguments.emplace_back(std::make_unique<VariableDecl>(
        source_range, member_name, std::move(type_node), nullptr));
  }

  auto ret_type_node = ASTConstructor::create_type_node(type, source_range);
  auto decl = std::make_unique<FunctionDecl>(source_range, constructor_name,
                                             std::move(arguments),
                                             std::move(ret_type_node), nullptr);
  decl->specifiers.set_extern(true);
  return std::move(decl);
}

}  // namespace Front
