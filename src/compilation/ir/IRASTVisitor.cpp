#include "IRASTVisitor.h"

IR::Type* Front::IRASTVisitor::map_type(Type* type) {
  auto map_int = [this](const IntType*) {
    // TODO: make some table with types widths
    return program_.types.make_type<IR::IntegerType>(4);
  };

  auto map_bool = [this](const BoolType*) {
    return program_.types.make_type<IR::BoolType>();
  };

  auto map_char = [this](const CharType*) {
    return program_.types.make_type<IR::IntegerType>(1);
  };

  auto map_pointer = [this](const PointerType*) {
    return program_.types.make_type<IR::PointerType>();
  };

  switch (type->get_kind()) {
    case Type::Kind::INT:
      return map_int(static_cast<IntType*>(type));
    case Type::Kind::BOOL:
      return map_bool(static_cast<BoolType*>(type));
    case Type::Kind::CHAR:
      return map_char(static_cast<CharType*>(type));
    case Type::Kind::POINTER:
      return map_pointer(static_cast<PointerType*>(type));
    case Type::Kind::FUNCTION:
      unreachable("Function type is not mapped into IR::Type");
  }

  unreachable("All type kinds are listed in switch above.");
}
