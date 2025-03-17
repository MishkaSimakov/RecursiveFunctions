#pragma once

#include <set>
#include <string>

namespace Front {
struct ModuleContext;
}

class StringId {
  using IteratorT = std::set<std::string>::const_iterator;
  IteratorT itr_;

  explicit StringId(IteratorT itr) : itr_(itr) {}

  friend struct Front::ModuleContext;

 public:
  StringId() = delete;

  bool operator==(StringId other) const { return other.itr_ == itr_; }

  size_t hash() const noexcept {
    // TODO: this is really bad, remove iterators hashing somehow
    return std::hash<const std::string*>()(&*itr_);
  }
};

template <>
struct std::hash<StringId> {
  size_t operator()(StringId id) const noexcept { return id.hash(); }
};
