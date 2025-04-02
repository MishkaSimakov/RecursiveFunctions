#include "SemanticAnalyzer.h"

namespace Front {

void SemanticAnalyzer::inject_symbol(ModuleContext& module,
                                     SymbolInfo& symbol) {
  const StringPool& external_strings = module.get_strings_pool();

  QualifiedId external_qualified_path = symbol.get_fully_qualified_name();
  StringId external_name = external_qualified_path.pop_name();
  Scope* external_scope = module.root_scope.get();

  auto local_qualified_path =
      import_external_string(external_qualified_path, external_strings);
  StringId local_name = import_external_string(external_name, external_strings);
  Scope* local_scope = context_.root_scope.get();

  size_t path_size = external_qualified_path.parts.size();

  for (size_t i = 0; i < path_size; ++i) {
    StringId external_part = external_qualified_path.parts[i];
    StringId local_part = local_qualified_path.parts[i];

    NamespaceSymbolInfo& external_namespace = std::get<NamespaceSymbolInfo>(
        external_scope->symbols.at(external_part));

    auto itr = local_scope->symbols.find(local_part);
    if (itr == local_scope->symbols.end()) {
      Scope* subscope = &local_scope->add_child(local_part);
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

  auto itr = local_scope->symbols.find(local_name);
  if (itr != local_scope->symbols.end()) {
    Declaration& decl = itr->second.get_declaration();
    scold_user(decl, "name is conflicting with name from module {:?}",
               module.name);
  }

  std::visit(
      Overloaded{
          [&](const VariableSymbolInfo& var) {
            Type* var_ty = inject_type(var.type, external_strings);
            local_scope->add_variable(local_name, var.declaration, var_ty);
          },
          [&](const NamespaceSymbolInfo& nmsp) {
            not_implemented("exported namespaces are not implemented");
          },
          [&](const FunctionSymbolInfo& fun) {
            Type* fun_ty = inject_type(fun.type, external_strings);
            local_scope->add_function(local_name, fun.declaration,
                                      static_cast<FunctionType*>(fun_ty),
                                      fun.subscope);
          },
          [&](const TypeAliasSymbolInfo& alias) {
            AliasType* alias_ty = static_cast<AliasType*>(
                inject_type(alias.type, external_strings));
            local_scope->symbols.emplace(
                local_name,
                TypeAliasSymbolInfo{local_scope, alias.declaration, alias_ty});
          },
          [&](const ClassSymbolInfo& cls) {
            ClassType* cls_ty = static_cast<ClassType*>(
                inject_type(cls.type, external_strings));
            auto cls_info =
                ClassSymbolInfo{local_scope, cls.subscope, cls.declaration};
            cls_info.type = cls_ty;
            local_scope->symbols.emplace(local_name, cls_info);
          }},
      symbol);
}

Type* SemanticAnalyzer::inject_type(Type* external_type,
                                    const StringPool& external_strings) {
  switch (external_type->get_kind()) {
    case Type::Kind::SIGNED_INT:
    case Type::Kind::UNSIGNED_INT:
    case Type::Kind::BOOL:
    case Type::Kind::CHAR: {
      size_t width = static_cast<PrimitiveType*>(external_type)->width;
      return types().add_primitive(external_type->get_kind(), width);
    }
    case Type::Kind::POINTER: {
      auto* pointer = static_cast<PointerType*>(external_type);
      return types().add_pointer(inject_type(pointer->child, external_strings));
    }
    case Type::Kind::FUNCTION: {
      auto* function = static_cast<FunctionType*>(external_type);
      auto local_arguments =
          function->arguments |
          std::views::transform([this, &external_strings](Type* arg_type) {
            return inject_type(arg_type, external_strings);
          });
      Type* local_return_type =
          inject_type(function->return_type, external_strings);
      return types().make_type<FunctionType>(
          std::vector(local_arguments.begin(), local_arguments.end()),
          local_return_type);
    }
    case Type::Kind::ALIAS: {
      AliasType* alias = static_cast<AliasType*>(external_type);
      auto* original = inject_type(alias->original, external_strings);
      QualifiedId local_name =
          import_external_string(alias->name, external_strings);

      return types().make_type<AliasType>(std::move(local_name), original);
    }
    case Type::Kind::TUPLE: {
      TupleType* tuple = static_cast<TupleType*>(external_type);
      std::vector<Type*> imported_elements;
      for (Type* type : tuple->elements) {
        imported_elements.push_back(inject_type(type, external_strings));
      }
      return types().make_type<TupleType>(std::move(imported_elements));
    }
    case Type::Kind::CLASS: {
      ClassType* external_cls = static_cast<ClassType*>(external_type);
      // for now, we make a deep copy of a class
      ClassType* local_cls = types().make_type<ClassType>(
          import_external_string(external_cls->name, external_strings));

      for (auto [name, type] : external_cls->members) {
        local_cls->members.emplace_back(
            import_external_string(name, external_strings),
            inject_type(type, external_strings));
      }

      return local_cls;
    }
    default:
      unreachable("all cases are enumerated above");
  }
}

SymbolInfo* SemanticAnalyzer::name_lookup(Scope* scope, const QualifiedId& id) {
  Scope* current_scope = scope;

  while (current_scope != nullptr &&
         !current_scope->has_symbol(id.parts.front())) {
    current_scope = current_scope->parent;
  }

  if (current_scope == nullptr) {
    return nullptr;
  }

  // TODO: handle undefined symbols
  for (StringId part : id.parts | std::views::take(id.parts.size() - 1)) {
    NamespaceSymbolInfo& namespace_info =
        std::get<NamespaceSymbolInfo>(current_scope->symbols.at(part));
    current_scope = namespace_info.subscope;
  }

  return &current_scope->symbols.at(id.parts.back());
}

}  // namespace Front
