#pragma once

#include <set>
#include <string>

class StringId {
  using IteratorT = std::set<std::string>::const_iterator;
  IteratorT itr_;

  explicit StringId(IteratorT itr) : itr_(itr) {}

  friend class StringPool;

 public:
  StringId() = delete;

  bool operator==(const StringId& other) const { return itr_ == other.itr_; }

  size_t hash() const noexcept {
    // TODO: this is really bad, remove iterators hashing somehow
    return std::hash<const std::string*>()(&*itr_);
  }
};

template <>
struct std::hash<StringId> {
  size_t operator()(StringId id) const noexcept { return id.hash(); }
};
