#pragma once

#include "ast/Nodes.h"
#include "utils/PointersStorage.h"

namespace Front {
class TypesStorage {
  PointersStorage<Type> storage_;

 public:
  template <typename T, typename... Args>
  T* make_type(Args&&... args) {
    return storage_.get_emplace<T>(std::forward<Args>(args)...);
  }

  PointerType* add_pointer(Type* type) {
    return storage_.get_emplace<PointerType>(type);
  }

  template <typename T>
    requires std::is_base_of_v<PrimitiveType, T>
  T* add_primitive(size_t width) {
    return storage_.get_emplace<T>(width);
  }

  PrimitiveType* add_primitive(Type::Kind type_kind, size_t width) {
    switch (type_kind) {
      case Type::Kind::SIGNED_INT:
        return add_primitive<SignedIntType>(width);
      case Type::Kind::UNSIGNED_INT:
        return add_primitive<UnsignedIntType>(width);
      case Type::Kind::BOOL:
        return add_primitive<BoolType>(width);
      case Type::Kind::CHAR:
        return add_primitive<CharType>(width);
      default:
        throw std::runtime_error("`type_kind` is not primitive.");
    }
  }

  AliasType* add_alias(QualifiedId name, Type* type) {
    Type* alias = storage_.get_emplace<AliasType>(std::move(name), type);
    return static_cast<AliasType*>(alias);
  }

  auto types_cbegin() const { return storage_.cbegin(); }
  auto types_cend() const { return storage_.cend(); }
};
}  // namespace Front
