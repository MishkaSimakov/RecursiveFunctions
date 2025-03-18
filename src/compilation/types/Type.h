#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ranges>
#include <typeinfo>
#include <vector>

#include "utils/Hashers.h"

namespace Front {
struct Type {
  constexpr static size_t kHashSeed = 123;

  enum class Kind { VOID, INT, BOOL, CHAR, POINTER, FUNCTION };

  virtual bool operator==(const Type&) const = 0;
  virtual size_t hash() const = 0;
  virtual std::string to_string() const = 0;
  virtual Kind get_kind() const = 0;

  virtual ~Type() = default;
};

struct PrimitiveType : Type {
  size_t hash() const override {
    return static_cast<size_t>(get_kind()) + kHashSeed;
  }

  bool operator==(const Type& other) const override {
    return typeid(other) == typeid(*this);
  }
};

struct VoidType final : PrimitiveType {
  std::string to_string() const override { return "void"; }
  Kind get_kind() const override { return Kind::VOID; }
};

struct IntType final : PrimitiveType {
  std::string to_string() const override { return "int"; }
  Kind get_kind() const override { return Kind::INT; }
};

struct BoolType final : PrimitiveType {
  std::string to_string() const override { return "bool"; }
  Kind get_kind() const override { return Kind::BOOL; }
};

struct CharType final : PrimitiveType {
  std::string to_string() const override { return "char"; }
  Kind get_kind() const override { return Kind::CHAR; }
};

struct PointerType final : Type {
  Type* child;

  bool operator==(const Type& other) const override {
    const PointerType* other_ptr = dynamic_cast<const PointerType*>(&other);
    if (other_ptr == nullptr) {
      return false;
    }

    return child == other_ptr->child;
  }

  size_t hash() const override {
    return tuple_hasher_fn(static_cast<size_t>(get_kind()) + kHashSeed,
                           child->hash());
  }
  std::string to_string() const override { return child->to_string() + "*"; }
  Kind get_kind() const override { return Kind::POINTER; }

  explicit PointerType(Type* child) : child(child) {}
};

struct FunctionType final : Type {
  std::vector<Type*> arguments;
  Type* return_type;

  bool operator==(const Type& other) const override {
    const FunctionType* other_ptr = dynamic_cast<const FunctionType*>(&other);
    if (other_ptr == nullptr) {
      return false;
    }

    return arguments == other_ptr->arguments &&
           return_type == other_ptr->return_type;
  }

  size_t hash() const override {
    StreamHasher hasher{};
    hasher << (static_cast<size_t>(get_kind()) + kHashSeed);

    for (const Type* argument : arguments) {
      hasher << argument->hash();
    }
    hasher << return_type->hash();

    return hasher.get_hash();
  }

  std::string to_string() const override {
    auto arguments_view =
        arguments | std::views::transform(
                        [](const Type* type) { return type->to_string(); });
    return fmt::format("({}) -> {}", fmt::join(arguments_view, ", "),
                       return_type->to_string());
  }

  Kind get_kind() const override { return Kind::FUNCTION; }

  FunctionType(std::vector<Type*> arguments, Type* return_type)
      : arguments(std::move(arguments)), return_type(return_type) {}
};
}  // namespace Front
