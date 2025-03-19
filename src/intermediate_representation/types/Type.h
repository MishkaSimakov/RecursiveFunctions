#pragma once

#include "utils/Hashers.h"

namespace IR {
struct Type {
  virtual size_t hash() const = 0;
  virtual ~Type() = default;
};

struct IntegerType : Type {
  size_t bytes;
  size_t hash() const override { return tuple_hasher_fn(0, bytes); }
};

struct BoolType : Type {
  size_t hash() const override { return tuple_hasher_fn(0); }
};

struct PointerType : Type {
  size_t hash() const override { return tuple_hasher_fn(1); }
};
}  // namespace IR
