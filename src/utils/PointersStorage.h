#pragma once

#include <unordered_set>

template <typename T>
  requires requires(T* ptr) {
    { ptr->hash() } -> std::same_as<size_t>;
  }
class PointersStorage {
  struct PolyPointerHasher {
    size_t operator()(T* ptr) const { return ptr->hash(); }
  };

  struct PolyPointerEqual {
    bool operator()(T* left, T* right) const { return *left == *right; }
  };

  std::unordered_set<T*, PolyPointerHasher, PolyPointerEqual> storage_;

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

  auto cbegin() const { return storage_.cbegin(); }
  auto cend() const { return storage_.cend(); }

  ~PointersStorage() {
    for (auto value : storage_) {
      delete value;
    }
  }
};
