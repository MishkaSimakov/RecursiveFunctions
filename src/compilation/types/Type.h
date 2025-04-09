#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ranges>
#include <typeinfo>
#include <vector>

#include "compilation/QualifiedId.h"
#include "utils/Hashers.h"
#include "utils/SmartEnum.h"
#include "utils/StringPool.h"

namespace Front {
struct Type {
 protected:
  constexpr static size_t kHashSeed = 123;

  StreamHasher init_hasher() const {
    StreamHasher hasher{};
    hasher << (static_cast<size_t>(get_kind()) + kHashSeed);
    return hasher;
  }

 public:
  ENUM(Kind, SIGNED_INT, UNSIGNED_INT, BOOL, CHAR, POINTER, TUPLE, FUNCTION,
       ALIAS, CLASS, NULLPTR);

  virtual bool operator==(const Type&) const = 0;
  virtual size_t hash() const = 0;
  virtual std::string to_string(const StringPool& strings) const = 0;
  virtual Kind get_kind() const = 0;

  bool is_primitive() const {
    return get_kind()
        .in<Kind::SIGNED_INT, Kind::UNSIGNED_INT, Kind::BOOL, Kind::CHAR>();
  }

  bool is_arithmetic() const {
    return get_kind().in<Kind::SIGNED_INT, Kind::UNSIGNED_INT, Kind::CHAR>();
  }

  bool is_passed_by_value() const {
    return get_original()->is_primitive() || get_kind().in<Kind::POINTER>() ||
           is_unit();
  }

  bool is_structural() const {
    return get_original()->get_kind().in<Kind::TUPLE, Kind::CLASS>();
  }

  bool is_unit() const;

  virtual Type* get_original() { return this; }
  virtual const Type* get_original() const { return this; }

  virtual ~Type() = default;
};

struct PrimitiveType : Type {
  const size_t width;

  explicit PrimitiveType(size_t width) : width(width) {}

  size_t hash() const override {
    return static_cast<size_t>(get_kind()) + width + kHashSeed;
  }

  bool operator==(const Type& other) const override {
    auto ptype = dynamic_cast<const PrimitiveType*>(&other);
    if (ptype == nullptr) {
      return false;
    }

    return typeid(other) == typeid(*this) && width == ptype->width;
  }
};

struct SignedIntType final : PrimitiveType {
  using PrimitiveType::PrimitiveType;

  std::string to_string(const StringPool& strings) const override {
    return fmt::format("i{}", width);
  }
  Kind get_kind() const override { return Kind::SIGNED_INT; }
};

struct UnsignedIntType final : PrimitiveType {
  using PrimitiveType::PrimitiveType;

  std::string to_string(const StringPool& strings) const override {
    return fmt::format("u{}", width);
  }
  Kind get_kind() const override { return Kind::UNSIGNED_INT; }
};

struct BoolType final : PrimitiveType {
  using PrimitiveType::PrimitiveType;

  std::string to_string(const StringPool& strings) const override {
    return fmt::format("b{}", width);
  }
  Kind get_kind() const override { return Kind::BOOL; }
};

struct CharType final : PrimitiveType {
  using PrimitiveType::PrimitiveType;

  std::string to_string(const StringPool& strings) const override {
    return fmt::format("c{}", width);
  }
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
    auto hasher = init_hasher();
    hasher << static_cast<const void*>(child);
    return hasher.get_hash();
  }
  std::string to_string(const StringPool& strings) const override {
    return child->to_string(strings) + "*";
  }
  Kind get_kind() const override { return Kind::POINTER; }

  explicit PointerType(Type* child) : child(child) {}
};

struct NullptrType final : Type {
  bool operator==(const Type& other) const override {
    const NullptrType* other_ptr = dynamic_cast<const NullptrType*>(&other);
    return other_ptr != nullptr;
  }

  size_t hash() const override {
    auto hasher = init_hasher();
    return hasher.get_hash();
  }
  std::string to_string(const StringPool& strings) const override {
    return "nullptr_t";
  }
  Kind get_kind() const override { return Kind::NULLPTR; }

  NullptrType() = default;
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
    auto hasher = init_hasher();
    for (const Type* argument : arguments) {
      hasher << static_cast<const void*>(argument);
    }
    hasher << static_cast<const void*>(return_type);

    return hasher.get_hash();
  }

  std::string to_string(const StringPool& strings) const override {
    auto arguments_view =
        arguments | std::views::transform([&strings](const Type* type) {
          return type->to_string(strings);
        });
    return fmt::format("({}) -> {}", fmt::join(arguments_view, ", "),
                       return_type->to_string(strings));
  }

  Kind get_kind() const override { return Kind::FUNCTION; }

  FunctionType(std::vector<Type*> arguments, Type* return_type)
      : arguments(std::move(arguments)), return_type(return_type) {}
};

struct AliasType final : Type {
  QualifiedId name;
  Type* original;

  bool operator==(const Type& other) const override {
    const AliasType* other_ptr = dynamic_cast<const AliasType*>(&other);
    if (other_ptr == nullptr) {
      return false;
    }

    return original == other_ptr->original && name == other_ptr->name;
  }

  Type* get_original() override { return original; }
  const Type* get_original() const override { return original; }

  size_t hash() const override {
    auto hasher = init_hasher();
    hasher << name;
    return hasher.get_hash();
  }

  std::string to_string(const StringPool& strings) const override {
    std::string original_str = original->to_string(strings);
    std::string name_str = name.to_string(strings);
    return fmt::format("{}(={})", name_str, original_str);
  }

  Kind get_kind() const override { return Kind::ALIAS; }

  AliasType(QualifiedId name, Type* original)
      : name(std::move(name)), original(original) {}
};

struct StructuralType : Type {
  virtual Type* get_member_type(size_t index) const = 0;
};

struct ClassType final : StructuralType {
  QualifiedId name;
  std::vector<std::pair<StringId, Type*>> members;

  bool operator==(const Type& other) const override {
    const ClassType* other_ptr = dynamic_cast<const ClassType*>(&other);
    if (other_ptr == nullptr) {
      return false;
    }

    return name == other_ptr->name;
  }

  size_t hash() const override {
    auto hasher = init_hasher();
    hasher << name;
    return hasher.get_hash();
  }

  std::string to_string(const StringPool& strings) const override {
    std::string name_str = name.to_string(strings);
    return fmt::format("{}", name_str);
  }

  Kind get_kind() const override { return Kind::CLASS; }

  Type* get_member_type(size_t index) const override {
    return members[index].second;
  }

  explicit ClassType(QualifiedId name) : name(std::move(name)) {}
};

struct TupleType final : StructuralType {
  std::vector<Type*> elements;

  bool operator==(const Type& other) const override {
    const TupleType* other_ptr = dynamic_cast<const TupleType*>(&other);
    if (other_ptr == nullptr) {
      return false;
    }

    return elements == other_ptr->elements;
  }

  size_t hash() const override {
    auto hasher = init_hasher();
    for (const Type* child : elements) {
      hasher << static_cast<const void*>(child);
    }
    return hasher.get_hash();
  }

  std::string to_string(const StringPool& strings) const override {
    auto elements_view =
        elements | std::views::transform([&strings](const Type* type) {
          return type->to_string(strings);
        });
    return fmt::format("({})", fmt::join(elements_view, ", "));
  }

  Kind get_kind() const override { return Kind::TUPLE; }

  Type* get_member_type(size_t index) const override { return elements[index]; }

  explicit TupleType(std::vector<Type*> elements)
      : elements(std::move(elements)) {}
};
}  // namespace Front
