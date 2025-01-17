#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "utils/Hashers.h"

struct Type {
  constexpr static bool is_primitive = false;

  virtual bool operator==(const Type&) const = 0;
  virtual size_t hash() const = 0;
  virtual std::string to_string() const = 0;

  virtual ~Type() = default;
};

struct IntType final : Type {
  constexpr static bool is_primitive = true;

  bool operator==(const Type& other) const override {
    const IntType* other_ptr = dynamic_cast<const IntType*>(&other);
    return other_ptr != nullptr;
  }

  size_t hash() const override { return 0; }

  std::string to_string() const override { return "int"; }
};

struct BoolType final : Type {
  constexpr static bool is_primitive = true;

  bool operator==(const Type& other) const override {
    const BoolType* other_ptr = dynamic_cast<const BoolType*>(&other);
    return other_ptr != nullptr;
  }

  size_t hash() const override { return 1; }

  std::string to_string() const override { return "bool"; }
};

struct CharType final : Type {
  constexpr static bool is_primitive = true;

  bool operator==(const Type& other) const override {
    const CharType* other_ptr = dynamic_cast<const CharType*>(&other);
    return other_ptr != nullptr;
  }

  size_t hash() const override { return 2; }

  std::string to_string() const override { return "char"; }
};

struct PointerType final : Type {
  constexpr static bool is_primitive = false;
  Type* child;

  bool operator==(const Type& other) const override {
    const PointerType* other_ptr = dynamic_cast<const PointerType*>(&other);
    if (other_ptr == nullptr) {
      return false;
    }

    return child == other_ptr->child;
  }

  size_t hash() const override { return tuple_hasher_fn(child->hash(), 0); }

  std::string to_string() const override { return child->to_string() + "*"; }

  explicit PointerType(Type* child) : child(child) {}
};

struct FunctionType final : Type {
  constexpr static bool is_primitive = false;
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

  FunctionType(std::vector<Type*> arguments, Type* return_type)
      : arguments(std::move(arguments)), return_type(return_type) {}
};
