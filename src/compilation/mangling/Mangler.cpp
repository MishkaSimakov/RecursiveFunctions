#include "Mangler.h"

#include <cassert>
#include <iostream>

#include "compilation/ModuleContext.h"

namespace Front {

std::string Mangler::get_substitution_name(size_t index) {
  if (index == 0) {
    return "S_";
  }

  return "S" + std::to_string(index - 1) + "_";
}

void Mangler::add_to_substitutions(std::string_view string) {
  substitutions_.emplace_back(string);
}

std::optional<std::string> Mangler::consider_substitution(
    std::string_view string) {
  for (size_t i = 0; i < substitutions_.size(); ++i) {
    if (substitutions_[i] == string) {
      return get_substitution_name(i);
    }
  }

  return std::nullopt;
}

std::string Mangler::substitutable(std::string string) {
  auto after_substitution = consider_substitution(string);
  if (after_substitution.has_value()) {
    return *after_substitution;
  }

  add_to_substitutions(string);
  return string;
}

std::string Mangler::mangle_type(Type* type) {
  type = type->get_original();

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

  std::string result;
  switch (type->get_kind()) {
    case Type::Kind::FUNCTION:
      return substitutable(
          "F" + mangle_bare_function_type(&type->as<FunctionType>()));
    case Type::Kind::BOOL:
      return "b";
    case Type::Kind::SIGNED_INT: {
      auto* int_type = static_cast<SignedIntType*>(type);
      return signed_int_mapping.at(int_type->get_width());
    }
    case Type::Kind::UNSIGNED_INT: {
      auto* int_type = static_cast<UnsignedIntType*>(type);
      return unsigned_int_mapping.at(int_type->get_width());
    }
    case Type::Kind::CHAR: {
      auto* char_type = static_cast<CharType*>(type);
      return char_mapping.at(char_type->get_width());
    }
    case Type::Kind::TUPLE: {
      auto& tuple_ty = type->as<TupleType>();
      if (tuple_ty.elements.empty()) {
        return "v";
      }

      constexpr auto kTupleTypeName = "tuple";
      // vendor-specific builtin extended type prefix
      std::string result = "u";
      result += mangle_source_name(kTupleTypeName);

      // template parameters prefix
      result += "I";

      for (Type* element : tuple_ty.elements) {
        result += mangle_type(element);
      }

      // template parameters suffix
      result += "E";
      return substitutable(result);
    }
    case Type::Kind::STRUCT: {
      auto& struct_ty = type->as<StructType>();
      // TODO: full name mangling
      return substitutable(mangle_name(struct_ty.get_name()));
    }
    case Type::Kind::POINTER: {
      auto& pointer_ty = type->as<PointerType>();
      return substitutable("P" + mangle_type(pointer_ty.get_child()));
    }

    default:
      not_implemented();
  }
}

std::string Mangler::mangle_bare_function_type(FunctionType* type) {
  std::string result;

  if (type->get_arguments().empty()) {
    result += "v";
  } else {
    for (Type* argument : type->get_arguments()) {
      result += mangle_type(argument);
    }
  }

  // "Non-template function names do not have return types encoded."

  return result;
}

std::string Mangler::mangle_source_name(StringId name) {
  std::string_view name_view = strings_.get_string(name);
  return mangle_source_name(name_view);
}

std::string Mangler::mangle_source_name(std::string_view name_view) {
  return std::to_string(name_view.size()) + std::string(name_view);
}

std::string Mangler::mangle_name(const QualifiedId& name) {
  if (name.parts.size() == 1) {
    return mangle_unscoped_name(name.parts.front());
  } else {
    return mangle_nested_name(name);
  }
}

std::string Mangler::mangle_unscoped_name(StringId name) {
  return mangle_source_name(name);
}

std::string Mangler::mangle_nested_name(const QualifiedId& name) {
  assert(name.is_qualified());

  std::string result;

  // prefix (participate in substitutions)
  for (StringId part : name.qualifiers_view()) {
    result += mangle_source_name(part);
    result = substitutable(result);
  }

  // unqualified name (don't participate in substitutions)
  result += mangle_source_name(name.parts.back());

  return "N" + result + "E";
}

std::string Mangler::mangle(const SymbolInfo& symbol) {
  substitutions_.clear();
  QualifiedId qualified_name = symbol.get_fully_qualified_name();

  if (symbol.is_function()) {
    // we mangle main function according to C mangling rules (just don't mangle)
    if (qualified_name.parts.size() == 1 &&
        strings_.get_string(qualified_name.parts.front()) == "main") {
      return "main";
    }

    FunctionType* function_type = symbol.as<FunctionSymbolInfo>().type;
    std::string result = "_Z" + mangle_name(qualified_name) +
                         mangle_bare_function_type(function_type);

    return result;
  }

  not_implemented();
}

}  // namespace Front
