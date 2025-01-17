#pragma once

#include <unordered_set>

#include "ast/Nodes.h"
#include "utils/Hashers.h"

class TypesStorage {
  constexpr static auto hasher_fn = [](Type* type_ptr) {
    return type_ptr->hash();
  };

  constexpr static auto equal_fn = [](Type* left, Type* right) {
    return *left == *right;
  };

  // type - modules where this type can be found
  std::unordered_set<Type*, decltype(hasher_fn), decltype(equal_fn)> types;

 public:
  template <typename T, typename... Args>
    requires std::is_base_of_v<Type, T> && std::is_constructible_v<T, Args...>
  auto get_or_make_type(Args&&... args) {
    T type(std::forward<Args>(args)...);
    auto itr = types.find(&type);

    if (itr != types.end()) {
      return itr;
    }

    T* allocated_type = new T(std::move(type));
    std::tie(itr, std::ignore) = types.emplace(allocated_type);
    return itr;
  }

  PointerType* add_pointer(Type* type) {
    Type* ptr_type = *get_or_make_type<PointerType>(type);
    return static_cast<PointerType*>(ptr_type);
  }

  template <typename T>
    requires std::is_base_of_v<Type, T> && T::is_primitive
  T* add_primitive() {
    Type* primitive_type = *get_or_make_type<T>();
    return static_cast<T*>(primitive_type);
  }

  ~TypesStorage();
};
