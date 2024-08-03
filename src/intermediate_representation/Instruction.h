#pragma once

#include <fmt/base.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <unordered_map>
#include <ranges>
#include <string>
#include <typeinfo>
#include <vector>

#include "Temporary.h"

namespace IR {
#define INSTRUCTION_ACCEPT_VISITOR()                        \
  void accept(InstructionVisitor& visitor) const override { \
    visitor.visit(*this);                                   \
  }
#define INSTRUCTION_VISITOR_VISIT(T) virtual void visit(const T&) = 0;

struct BasicBlock;
struct FunctionCall;
struct Addition;
struct Subtraction;
struct Move;
struct Phi;
struct Return;
struct Branch;
struct Load;
struct Store;

struct InstructionVisitor {
  INSTRUCTION_VISITOR_VISIT(FunctionCall);
  INSTRUCTION_VISITOR_VISIT(Addition);
  INSTRUCTION_VISITOR_VISIT(Subtraction);
  INSTRUCTION_VISITOR_VISIT(Move);
  INSTRUCTION_VISITOR_VISIT(Phi);
  INSTRUCTION_VISITOR_VISIT(Return);
  INSTRUCTION_VISITOR_VISIT(Branch);
  INSTRUCTION_VISITOR_VISIT(Load);
  INSTRUCTION_VISITOR_VISIT(Store);

  virtual ~InstructionVisitor();
};

template <typename T>
concept ContainsTemporary =
    std::same_as<T, Temporary> || std::same_as<T, TemporaryOrConstant>;

template <typename T>
concept RangeOrValueContainingTemporary =
    std::ranges::range<T> && ContainsTemporary<std::ranges::range_value_t<T>> ||
    ContainsTemporary<T>;

struct Instruction {
 private:
  virtual bool equal(const Instruction&) const = 0;

  template <typename T>
  static void filter_temporaries_inserter(std::vector<Temporary>& result,
                                          T&& value) {
    if constexpr (std::is_same_v<Temporary, std::decay_t<T>>) {
      result.emplace_back(value);
    } else if constexpr (std::is_same_v<TemporaryOrConstant, std::decay_t<T>>) {
      if (!value.is_constant) {
        result.emplace_back(Temporary{value.index()});
      }
    } else {
      for (auto temp : value) {
        filter_temporaries_inserter(result, temp);
      }
    }
  }

  template <typename T>
  static void replace_temporaries_helper(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map,
      T& value) {
    if constexpr (std::is_same_v<Temporary, T>) {
      auto itr = temp_map.find(value);

      if (itr != temp_map.end()) {
        value.index = itr->second.index();
      }
    } else if constexpr (std::is_same_v<TemporaryOrConstant, std::decay_t<T>>) {
      if (!value.is_constant) {
        auto itr = temp_map.find(Temporary{value.index()});

        if (itr != temp_map.end()) {
          value = itr->second;
        }
      }
    } else {
      for (auto& temp : value) {
        replace_temporaries_helper(temp_map, temp);
      }
    }
  }

  template <typename T>
    requires std::is_base_of_v<Instruction, T>
  bool is_of_type() const {
    return typeid(*this) == typeid(T);
  }

 protected:
  template <typename... Args>
    requires(RangeOrValueContainingTemporary<std::decay_t<Args>> && ...)
  static std::vector<Temporary> filter_temporaries(Args&&... args) {
    std::vector<Temporary> result;

    (filter_temporaries_inserter(result, std::forward<decltype(args)>(args)),
     ...);
    return result;
  }

  template <typename... Args>
    requires(RangeOrValueContainingTemporary<Args> && ...)
  static void filter_and_replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map,
      Args&... args) {
    (replace_temporaries_helper(temp_map, args), ...);
  }

 public:
  Temporary result_destination;

  Instruction() = default;

  explicit Instruction(Temporary result_destination)
      : result_destination(result_destination) {}

  virtual std::string to_string() const = 0;

  bool operator==(const Instruction& other) const {
    return typeid(*this) == typeid(other) && equal(other);
  }

  virtual std::vector<Temporary> get_temporaries_in_arguments() const = 0;

  bool has_return_value() const;

  virtual std::unique_ptr<Instruction> clone() const = 0;

  virtual void accept(InstructionVisitor& visitor) const = 0;

  virtual void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map) = 0;

  virtual ~Instruction() = default;
};

struct FunctionCall final : Instruction {
  std::string name;
  std::vector<TemporaryOrConstant> arguments;

  FunctionCall(Temporary result_destination, const std::string& name,
               const std::vector<TemporaryOrConstant>& arguments = {})
      : Instruction(result_destination), name(name), arguments(arguments) {}

  std::string to_string() const override {
    return fmt::format("{} = call {}({})", result_destination, name,
                       fmt::join(arguments | std::views::transform(
                                                 [](TemporaryOrConstant value) {
                                                   return value.to_string();
                                                 }),
                                 ", "));
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return filter_temporaries(arguments);
  }

  INSTRUCTION_ACCEPT_VISITOR();

  std::unique_ptr<Instruction> clone() const override {
    return std::make_unique<FunctionCall>(result_destination, name, arguments);
  }

  void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map)
      override {
    filter_and_replace_temporaries(temp_map, result_destination, arguments);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const FunctionCall&>(instruction);

    return name == other.name && arguments == other.arguments;
  }
};

struct Addition final : Instruction {
  TemporaryOrConstant left;
  TemporaryOrConstant right;

  Addition(Temporary result_destination, TemporaryOrConstant left,
           TemporaryOrConstant right)
      : Instruction(result_destination), left(left), right(right) {}

  std::string to_string() const override {
    return fmt::format("{} = add {} {}", result_destination, left, right);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return filter_temporaries(left, right);
  }

  INSTRUCTION_ACCEPT_VISITOR();

  std::unique_ptr<Instruction> clone() const override {
    return std::make_unique<Addition>(result_destination, left, right);
  }

  void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map)
      override {
    filter_and_replace_temporaries(temp_map, result_destination, left, right);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Addition&>(instruction);

    return left == other.left && right == other.right;
  }
};

struct Subtraction final : Instruction {
  TemporaryOrConstant left;
  TemporaryOrConstant right;

  Subtraction(Temporary result_destination, TemporaryOrConstant left,
              TemporaryOrConstant right)
      : Instruction(result_destination), left(left), right(right) {}

  std::string to_string() const override {
    return fmt::format("{} = sub {} {}", result_destination, left, right);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return filter_temporaries(left, right);
  }

  INSTRUCTION_ACCEPT_VISITOR();

  std::unique_ptr<Instruction> clone() const override {
    return std::make_unique<Subtraction>(result_destination, left, right);
  }

  void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map)
      override {
    filter_and_replace_temporaries(temp_map, result_destination, left, right);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Subtraction&>(instruction);

    return left == other.left && right == other.right;
  }
};

struct Move final : Instruction {
  TemporaryOrConstant source;

  Move(Temporary result_destination, TemporaryOrConstant source)
      : Instruction(result_destination), source(source) {}

  std::string to_string() const override {
    return fmt::format("{} = {}", result_destination, source);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return filter_temporaries(source);
  }

  INSTRUCTION_ACCEPT_VISITOR();

  std::unique_ptr<Instruction> clone() const override {
    return std::make_unique<Move>(result_destination, source);
  }

  void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map)
      override {
    filter_and_replace_temporaries(temp_map, result_destination, source);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Move&>(instruction);

    return source == other.source;
  }
};

struct Phi final : Instruction {
  // index of BB in function BBs vector / temporary from this block
  std::vector<std::pair<BasicBlock*, TemporaryOrConstant>> values;

  std::string to_string() const override {
    auto arguments = values | std::views::elements<1> |
                     std::views::transform([](TemporaryOrConstant value) {
                       return value.to_string();
                     });

    return fmt::format("{} = phi [{}]", result_destination,
                       fmt::join(arguments, ", "));
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    // return filter_temporaries(values);
    // TODO: think about it
    return {};
  }

  INSTRUCTION_ACCEPT_VISITOR();

  Phi(Temporary result_destination,
      const std::vector<std::pair<BasicBlock*, TemporaryOrConstant>>& values =
          {})
      : Instruction(result_destination), values(values) {}

  std::unique_ptr<Instruction> clone() const override {
    return std::make_unique<Phi>(result_destination, values);
  }

  void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map)
      override {
    filter_and_replace_temporaries(temp_map, result_destination);

    for (auto& temp : values | std::views::elements<1>) {
      filter_and_replace_temporaries(temp_map, temp);
    }
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Phi&>(instruction);

    return values == other.values;
  }
};

struct Return final : Instruction {
  TemporaryOrConstant value;

  std::string to_string() const override {
    return fmt::format("return {}", value);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return filter_temporaries(value);
  }

  INSTRUCTION_ACCEPT_VISITOR();

  Return(TemporaryOrConstant value) : value(value) {}

  std::unique_ptr<Instruction> clone() const override {
    return std::make_unique<Return>(value);
  }

  void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map)
      override {
    filter_and_replace_temporaries(temp_map, result_destination, value);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Return&>(instruction);
    return value == other.value;
  }
};

struct Branch final : Instruction {
  TemporaryOrConstant value;

  // TODO: some kind of condition must be here

  std::string to_string() const override {
    return fmt::format("branch {} == 0 ?", value);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return filter_temporaries(value);
  }

  INSTRUCTION_ACCEPT_VISITOR();

  Branch(TemporaryOrConstant value) : value(value) {}

  std::unique_ptr<Instruction> clone() const override {
    return std::make_unique<Branch>(value);
  }

  void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map)
      override {
    filter_and_replace_temporaries(temp_map, result_destination, value);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Branch&>(instruction);
    return value == other.value;
  }
};

struct Load final : Instruction {
  size_t index;

  std::string to_string() const override {
    return fmt::format("{} = load {}", result_destination, index);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return {};
  }

  INSTRUCTION_ACCEPT_VISITOR();

  Load(Temporary result_destination, size_t index)
      : Instruction(result_destination), index(index) {}

  std::unique_ptr<Instruction> clone() const override {
    return std::make_unique<Load>(result_destination, index);
  }

  void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map)
      override {
    filter_and_replace_temporaries(temp_map, result_destination);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Load&>(instruction);
    return index == other.index;
  }
};

struct Store final : Instruction {
  Temporary temporary;
  size_t index;

  std::string to_string() const override {
    return fmt::format("store {} into {}", temporary, index);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return {};
  }

  INSTRUCTION_ACCEPT_VISITOR();

  Store(Temporary temporary, size_t index)
      : temporary(temporary), index(index) {}

  std::unique_ptr<Instruction> clone() const override {
    return std::make_unique<Store>(temporary, index);
  }

  void replace_temporaries(
      const std::unordered_map<Temporary, TemporaryOrConstant>& temp_map)
      override {
    filter_and_replace_temporaries(temp_map, result_destination, temporary);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Store&>(instruction);
    return temporary == other.temporary && index == other.index;
  }
};

// this should be after because of class declarations and other stuff...
inline bool Instruction::has_return_value() const {
  return !(is_of_type<Return>() || is_of_type<Branch>() || is_of_type<Store>());
}

}  // namespace IR
