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
    Type* ptr_type = storage_.get_emplace<PointerType>(type);
    return static_cast<PointerType*>(ptr_type);
  }

  template <typename T>
    requires std::is_base_of_v<PrimitiveType, T>
  T* add_primitive() {
    Type* primitive_type = storage_.get_emplace<T>();
    return static_cast<T*>(primitive_type);
  }

  auto types_cbegin() const { return storage_.cbegin(); }
  auto types_cend() const { return storage_.cend(); }
};
}  // namespace Front
