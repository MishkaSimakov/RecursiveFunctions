#include "Mangler.h"

#include "compilation/ModuleContext.h"

namespace Front {

std::string Mangler::mangle_type(Type* type) const {
  const std::unordered_map<size_t, std::string> signed_int_mapping = {
      {8, "a"},
      {16, "s"},
      {32, "i"},
      {64, "x"},
  };
  const std::unordered_map<size_t, std::string> unsigned_int_mapping = {
      {8, "h"},
      {16, "t"},
      {32, "j"},
      {64, "y"},
  };
  const std::unordered_map<size_t, std::string> char_mapping = {
      {8, "c"},
      {16, "Ds"},
      {32, "Di"},
  };

  switch (type->get_kind()) {
    case Type::Kind::FUNCTION:
      return "F" + mangle_bare_function_type(static_cast<FunctionType*>(type));
    case Type::Kind::BOOL:
      return "b";
    case Type::Kind::SIGNED_INT: {
      auto* int_type = static_cast<SignedIntType*>(type);
      return signed_int_mapping.at(int_type->width);
    }
    case Type::Kind::UNSIGNED_INT: {
      auto* int_type = static_cast<UnsignedIntType*>(type);
      return unsigned_int_mapping.at(int_type->width);
    }
    case Type::Kind::CHAR: {
      auto* char_type = static_cast<CharType*>(type);
      return char_mapping.at(char_type->width);
    }
    case Type::Kind::TUPLE: {
      auto* tuple_ty = static_cast<TupleType*>(type);
      if (tuple_ty->elements.empty()) {
        return "v";
      }

      constexpr auto kTupleTypeName = "tuple";
      // vendor-specific builtin extended type prefix
      std::string result = "u";
      result += mangle_source_name(kTupleTypeName);

      // template parameters prefix
      result += "I";

      for (Type* element : tuple_ty->elements) {
        result += mangle_type(element);
      }

      // template parameters suffix
      result += "E";
      return result;
    }
    case Type::Kind::CLASS: {
      auto* class_ty = static_cast<ClassType*>(type);
      // TODO: full name mangling
      return mangle_source_name(class_ty->name.parts.back());
    }

    default:
      not_implemented();
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
  std::string_view name_view = strings_.get_string(name);
  return mangle_source_name(name_view);
}

std::string Mangler::mangle_source_name(std::string_view name_view) const {
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
        strings_.get_string(qualified_name.parts.front()) == "main") {
      return "main";
    }

    FunctionType* function_type = std::get<FunctionSymbolInfo>(symbol).type;
    return "_Z" + mangle_name(qualified_name) +
           mangle_bare_function_type(function_type);
  }

  not_implemented();
}

}  // namespace Front
