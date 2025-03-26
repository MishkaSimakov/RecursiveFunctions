#include "SemanticAnalyzer.h"

namespace Front {

void SemanticAnalyzer::inject_symbol(ModuleContext& module,
                                     SymbolInfo& symbol) {
  QualifiedId external_qualified_path = symbol.get_fully_qualified_name();
  StringId external_name = external_qualified_path.pop_name();
  Scope* external_scope = module.root_scope.get();

  auto local_qualified_path =
      import_external_string(external_qualified_path, module);
  StringId local_name = import_external_string(external_name, module);
  Scope* local_scope = context_.root_scope.get();

  size_t path_size = external_qualified_path.parts.size();

  for (size_t i = 0; i < path_size; ++i) {
    StringId external_part = external_qualified_path.parts[i];
    StringId local_part = local_qualified_path.parts[i];

    NamespaceSymbolInfo& external_namespace = std::get<NamespaceSymbolInfo>(
        external_scope->symbols.at(external_part));

    auto itr = local_scope->symbols.find(local_part);
    if (itr == local_scope->symbols.end()) {
      Scope* subscope = &local_scope->add_child();
      subscope->name = local_part;
      local_scope->add_namespace(local_part, external_namespace.declaration,
                                 subscope);

      local_scope = subscope;
    } else if (!std::holds_alternative<NamespaceSymbolInfo>(itr->second)) {
      throw std::runtime_error("Error!");
    } else {
      NamespaceSymbolInfo& local_namespace =
          std::get<NamespaceSymbolInfo>(itr->second);
      local_scope = local_namespace.subscope;
    }

    external_scope = external_namespace.subscope;
  }

  if (local_scope->has_symbol(local_name)) {
    throw std::runtime_error("Redeclaration of function!");
  }

  auto& function_info = std::get<FunctionSymbolInfo>(symbol);
  FunctionType* local_type =
      static_cast<FunctionType*>(inject_type(function_info.type));
  local_scope->add_function(local_name, function_info.declaration, local_type);
}

Type* SemanticAnalyzer::inject_type(Type* external_type) {
  switch (external_type->get_kind()) {
    case Type::Kind::VOID:
      return types().add_primitive<VoidType>();
    case Type::Kind::INT:
      return types().add_primitive<IntType>();
    case Type::Kind::BOOL:
      return types().add_primitive<BoolType>();
    case Type::Kind::CHAR:
      return types().add_primitive<CharType>();
    case Type::Kind::POINTER:
      return types().add_pointer(
          inject_type(static_cast<PointerType*>(external_type)->child));
    case Type::Kind::FUNCTION: {
      auto* function = static_cast<FunctionType*>(external_type);
      auto local_arguments =
          function->arguments | std::views::transform([this](Type* arg_type) {
            return inject_type(arg_type);
          });
      Type* local_return_type = inject_type(function->return_type);
      return types().make_type<FunctionType>(
          std::vector(local_arguments.begin(), local_arguments.end()),
          local_return_type);
    }
  }
}

SymbolInfo* SemanticAnalyzer::name_lookup(Scope* scope, const QualifiedId& id) {
  Scope* current_scope = scope;

  while (current_scope != nullptr &&
         !current_scope->has_symbol(id.parts.front())) {
    current_scope = current_scope->parent;
  }

  if (current_scope == nullptr) {
    throw std::runtime_error("Couldn't find symbol.");
  }

  for (StringId part : id.parts | std::views::take(id.parts.size() - 1)) {
    NamespaceSymbolInfo& namespace_info =
        std::get<NamespaceSymbolInfo>(current_scope->symbols.at(part));
    current_scope = namespace_info.subscope;
  }

  return &current_scope->symbols.at(id.parts.back());
}

}  // namespace Front
