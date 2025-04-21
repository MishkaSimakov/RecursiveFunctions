#include "SemanticAnalyzer.h"

namespace Front {

struct SymbolInjector {
  ModuleContext& source_module;
  Scope* scope;
  StringId name;

  SemanticAnalyzer& analyzer;

  StringPool& source_strings() { return source_module.get_strings_pool(); }

  void operator()(const VariableSymbolInfo& var) {
    Type* var_ty = analyzer.inject_type(var.type, source_strings());
    scope->add_variable(name, var.declaration, var_ty);
  }

  void operator()(const NamespaceSymbolInfo& nmsp) {
    Scope* subscope = &scope->add_child(name);
    scope->add_namespace(name, nmsp.declaration, subscope);

    for (auto& child : nmsp.subscope->symbols | std::views::values) {
      analyzer.inject_symbol_to(subscope, source_module, child);
    }
  }

  void operator()(const FunctionSymbolInfo& fun) {
    Type* fun_ty = analyzer.inject_type(fun.type, source_strings());
    FunctionSymbolInfo copy = fun;
    copy.type = &fun_ty->as<FunctionType>();
    copy.transformation_type =
        copy.transformation_type != nullptr
            ? analyzer.inject_type(copy.transformation_type, source_strings())
            : nullptr;

    scope->symbols.emplace(name, copy);
  }

  void operator()(const TypeAliasSymbolInfo& alias) {
    auto& type =
        analyzer.inject_type(alias.type, source_strings())->as<AliasType>();
    scope->symbols.emplace(
        name, TypeAliasSymbolInfo{scope, alias.declaration, &type});
  }

  void operator()(const StructSymbolInfo& cls) {
    Scope* subscope = &scope->add_child(name);
    auto info = StructSymbolInfo{scope, subscope, cls.declaration};

    info.type =
        &analyzer.inject_type(cls.type, source_strings())->as<StructType>();
    analyzer.context_.structs_info.emplace(
        info.type, scope->symbols.emplace(name, info).first->second);

    for (auto& child : cls.subscope->symbols | std::views::values) {
      analyzer.inject_symbol_to(subscope, source_module, child);
    }
  }
};

Scope* SemanticAnalyzer::inject_nested_scope(const ModuleContext& source_module,
                                             QualifiedId source_namespaces) {
  const StringPool& external_strings = source_module.get_strings_pool();

  QualifiedId external_qualified_path = source_namespaces;
  Scope* external_scope = source_module.root_scope.get();

  auto local_qualified_path =
      import_external_string(external_qualified_path, external_strings);
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
    } else if (!itr->second.is_namespace()) {
      throw std::runtime_error("Error!");
    } else {
      NamespaceSymbolInfo& local_namespace =
          std::get<NamespaceSymbolInfo>(itr->second);
      local_scope = local_namespace.subscope;
    }

    external_scope = external_namespace.subscope;
  }

  return local_scope;
}

void SemanticAnalyzer::inject_symbol(ModuleContext& source_module,
                                     SymbolInfo& source_symbol) {
  QualifiedId qualifiers = source_symbol.get_fully_qualified_name();
  qualifiers.pop_name();

  Scope* scope = inject_nested_scope(source_module, std::move(qualifiers));
  inject_symbol_to(scope, source_module, source_symbol);
}

void SemanticAnalyzer::inject_symbol_to(Scope* scope,
                                        ModuleContext& source_module,
                                        SymbolInfo& source_symbol) {
  StringId name = import_external_string(source_symbol.get_unqualified_name(),
                                         source_module.get_strings_pool());

  auto itr = scope->symbols.find(name);
  if (itr != scope->symbols.end()) {
    Declaration& decl = itr->second.get_declaration();
    scold_user(decl, "name is conflicting with name from module {:?}",
               source_module.name);
  }

  SymbolInjector injector{source_module, scope, name, *this};
  std::visit(injector, source_symbol);
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
    case Type::Kind::STRUCT: {
      StructType* external_cls = static_cast<StructType*>(external_type);
      // for now, we make a deep copy of a class
      StructType* local_cls = types().make_type<StructType>(
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

  for (StringId part : id.qualifiers_view()) {
    auto itr = current_scope->symbols.find(part);

    if (itr == current_scope->symbols.end()) {
      return nullptr;
    }

    SymbolInfo& current_symbol = itr->second;
    if (current_symbol.is_namespace()) {
      current_scope = std::get<NamespaceSymbolInfo>(current_symbol).subscope;
    } else if (current_symbol.is_class()) {
      current_scope = std::get<StructSymbolInfo>(current_symbol).subscope;
    } else {
      // TODO: better error handling here
      return nullptr;
    }
  }

  auto itr = current_scope->symbols.find(id.unqualified_id());

  return itr != current_scope->symbols.end() ? &itr->second : nullptr;
}

}  // namespace Front
