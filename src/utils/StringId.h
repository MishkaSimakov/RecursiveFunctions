#pragma once

#include <cassert>
#include <set>
#include <string>

class StringPool;

namespace detail {

template <bool ShouldExist>
struct StringPoolIdentifier {
  explicit StringPoolIdentifier(StringPool* pool) {}

  bool operator==(const StringPoolIdentifier&) const = default;
};

template <>
struct StringPoolIdentifier<true> {
 private:
  const StringPool* pool_;

 public:
  explicit StringPoolIdentifier(const StringPool* pool) : pool_(pool) {}

  bool operator==(const StringPoolIdentifier&) const = default;
};

}  // namespace detail

class StringId {
  using IteratorT = std::set<std::string>::const_iterator;
  IteratorT itr_;

  detail::StringPoolIdentifier<Constants::debug> pool_identifier_;

  explicit StringId(IteratorT itr, StringPool* pool)
      : itr_(itr), pool_identifier_(pool) {}

  friend class StringPool;

 public:
  StringId() = delete;

  bool operator==(const StringId& other) const {
    if constexpr (Constants::debug) {
      assert(pool_identifier_ == other.pool_identifier_);
    }

    return itr_ == other.itr_;
  }

  size_t hash() const noexcept {
    // TODO: this is really bad, remove iterators hashing somehow
    return std::hash<const std::string*>()(&*itr_);
  }
};

template <>
struct std::hash<StringId> {
  size_t operator()(StringId id) const noexcept { return id.hash(); }
};
