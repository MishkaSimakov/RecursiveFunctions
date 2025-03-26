#include "Mangler.h"

#include "compilation/ModuleContext.h"

namespace Front {

std::string Mangler::mangle_type(Type* type) const {
  switch (type->get_kind()) {
    case Type::Kind::FUNCTION:
      return "F" + mangle_bare_function_type(static_cast<FunctionType*>(type));
    case Type::Kind::BOOL:
      return "b";
    case Type::Kind::INT:
      return "x";
    default:
      throw std::runtime_error("Not implemented.");
  }
}

std::string Mangler::mangle_bare_function_type(FunctionType* type) const {
  std::string result;

  if (type->arguments.empty()) {
    result += "v";
  } else {
    for (Type* argument : type->arguments) {
      result += mangle_type(argument);
    }
  }

  // "Non-template function names do not have return types encoded."

  return result;
}

std::string Mangler::mangle_source_name(StringId name) const {
  std::string_view name_view = context_.get_string(name);
  return std::to_string(name_view.size()) + std::string(name_view);
}

std::string Mangler::mangle_name(const QualifiedId& name) const {
  std::string result;
  bool is_nested = name.parts.size() > 1;

  if (is_nested) {
    result += "N";
  }

  for (StringId part : name.parts) {
    result += mangle_source_name(part);
  }

  if (is_nested) {
    result += "E";
  }

  return result;
}

std::string Mangler::mangle(const SymbolInfo& symbol) const {
  QualifiedId qualified_name = symbol.get_fully_qualified_name();

  if (symbol.is_function()) {
    // we mangle main function according to C mangling rules (just don't mangle)
    if (qualified_name.parts.size() == 1 &&
        context_.get_string(qualified_name.parts.front()) == "main") {
      return "main";
    }

    FunctionType* function_type = std::get<FunctionSymbolInfo>(symbol).type;
    return "_Z" + mangle_name(qualified_name) +
           mangle_bare_function_type(function_type);
  }

  throw std::runtime_error("Not implemented.");
}

}  // namespace Front
