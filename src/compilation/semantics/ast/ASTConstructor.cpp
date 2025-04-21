#include "ASTConstructor.h"

#include "errors/Helpers.h"

namespace Front {

std::unique_ptr<TypeNode> ASTConstructor::create_type_node(
    Type* type, SourceRange source_range) {
  Type::Kind kind = type->get_kind();
  switch (kind) {
    case Type::Kind::SIGNED_INT:
    case Type::Kind::UNSIGNED_INT:
    case Type::Kind::BOOL:
    case Type::Kind::CHAR: {
      auto* primitive_type = static_cast<PrimitiveType*>(type);
      return std::make_unique<PrimitiveTypeNode>(source_range, kind,
                                                 primitive_type->get_width());
    }
    case Type::Kind::POINTER: {
      auto* pointer_type = static_cast<PointerType*>(type);
      return std::make_unique<PointerTypeNode>(
          source_range, create_type_node(pointer_type->get_child(), source_range));
    }
    case Type::Kind::TUPLE: {
      auto* tuple_type = static_cast<TupleType*>(type);
      std::vector<std::unique_ptr<TypeNode>> children;
      for (auto* element : tuple_type->elements) {
        children.push_back(create_type_node(element, source_range));
      }
      return std::make_unique<TupleTypeNode>(source_range, std::move(children));
    }
    case Type::Kind::FUNCTION: {
      // Function types are not expected in this context for TeaLang's AST.
      // If needed, a FunctionTypeNode could be added in the future.
      unreachable("Function types are not supported in TypeNode construction.");
    }
    case Type::Kind::ALIAS: {
      auto* alias_type = static_cast<AliasType*>(type);
      return std::make_unique<UserDefinedTypeNode>(source_range,
                                                   alias_type->get_name());
    }
    case Type::Kind::STRUCT: {
      auto* class_type = static_cast<StructType*>(type);
      return std::make_unique<UserDefinedTypeNode>(source_range,
                                                   class_type->get_name());
    }
    default:
      unreachable("Unknown Type::Kind encountered.");
  }
}

}  // namespace Front
