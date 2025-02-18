#pragma once

#include <unordered_set>

template <typename T>
  requires requires(T* ptr) {
    { ptr->hash() } -> std::same_as<size_t>;
  }
class PointersStorage {
  constexpr static auto hasher_fn = [](T* ptr) { return ptr->hash(); };

  constexpr static auto equal_fn = [](T* left, T* right) {
    return *left == *right;
  };

  std::unordered_set<T*, decltype(hasher_fn), decltype(equal_fn)> storage_;

 public:
  template <typename Child, typename... Args>
    requires std::is_base_of_v<T, Child> &&
             std::is_constructible_v<Child, Args...>
  Child* get_emplace(Args&&... args) {
    Child value(std::forward<Args>(args)...);
    return get_push(std::move(value));
  }

  template <typename Child>
    requires std::is_base_of_v<T, Child>
  Child* get_push(Child&& value) {
    auto itr = storage_.find(&value);

    if (itr != storage_.end()) {
      return static_cast<Child*>(*itr);
    }

    Child* allocated = new Child(std::move(value));
    std::tie(itr, std::ignore) = storage_.emplace(allocated);
    return allocated;
  }

  ~PointersStorage() {
    for (auto value : storage_) {
      delete value;
    }
  }
};
