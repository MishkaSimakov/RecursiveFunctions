#pragma once
#include <utils/PointersStorage.h>

#include "Type.h"

namespace IR {
class TypesStorage {
  PointersStorage<Type> storage_;

 public:
  template <typename T, typename... Args>
  T* make_type(Args&&... args) {
    return storage_.get_emplace<T>(std::forward<Args>(args)...);
  }
};
}  // namespace IR
