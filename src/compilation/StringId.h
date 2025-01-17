#pragma once

#include <unordered_map>

class StringId {
  size_t id_;

  explicit StringId(size_t id) : id_(id) {}

  friend struct GlobalContext;

 public:
  bool operator==(StringId other) const { return other.id_ == id_; }

  size_t hash() const noexcept { return std::hash<size_t>()(id_); }
};

template <>
struct std::hash<StringId> {
  size_t operator()(StringId id) const noexcept { return id.hash(); }
};
