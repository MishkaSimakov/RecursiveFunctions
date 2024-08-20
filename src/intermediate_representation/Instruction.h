#pragma once

#include <fmt/base.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <ranges>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "Value.h"

template <bool is_empty, typename T>
struct PossiblyEmptyStorage {};

template <typename T>
struct PossiblyEmptyStorage<false, T> {
  T value;

  explicit PossiblyEmptyStorage(T value) : value(value) {}

  T& operator=(const T& other) {
    value = other;
    return value;
  }

  bool operator==(const PossiblyEmptyStorage&) const = default;

  operator T() { return value; }
  operator T() const { return value; }

  bool operator==(const T& other) const { return value == other; }
};

// fmt library helpers
template <bool is_empty, typename T>
struct fmt::formatter<PossiblyEmptyStorage<is_empty, T>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(PossiblyEmptyStorage<is_empty, T> value,
              FormatContext& ctx) const {
    if constexpr (is_empty) {
      return fmt::format_to(ctx.out(), "empty");
    } else {
      return fmt::format_to(ctx.out(), "{}", value.value);
    }
  }
};

namespace IR {
#define INSTRUCTION_MEMBERS()                                      \
  void accept(InstructionVisitor& visitor) const override {        \
    visitor.visit(*this);                                          \
  }                                                                \
  std::unique_ptr<BaseInstruction> clone() const override {        \
    return std::make_unique<std::decay_t<decltype(*this)>>(*this); \
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
struct Jump;
struct Load;
struct Store;
struct Select;

struct InstructionVisitor {
  INSTRUCTION_VISITOR_VISIT(FunctionCall);
  INSTRUCTION_VISITOR_VISIT(Addition);
  INSTRUCTION_VISITOR_VISIT(Subtraction);
  INSTRUCTION_VISITOR_VISIT(Move);
  INSTRUCTION_VISITOR_VISIT(Phi);
  INSTRUCTION_VISITOR_VISIT(Return);
  INSTRUCTION_VISITOR_VISIT(Branch);
  INSTRUCTION_VISITOR_VISIT(Jump);
  INSTRUCTION_VISITOR_VISIT(Load);
  INSTRUCTION_VISITOR_VISIT(Store);
  INSTRUCTION_VISITOR_VISIT(Select);

  virtual ~InstructionVisitor() = default;
};

struct BaseInstruction {
 private:
  template <typename T>
  static void replace_values_helper_helper(
      const std::unordered_map<Value, Value>& mapping, T& arg) {
    if constexpr (std::is_same_v<Value, T>) {
      auto itr = mapping.find(arg);

      if (itr != mapping.end()) {
        arg = itr->second;
      }
    } else {
      for (Value& value : arg) {
        replace_values_helper_helper(mapping, value);
      }
    }
  }

 protected:
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, Value>
  static std::vector<Value> filter_arguments_helper(R&& range, ValueType type) {
    std::vector<Value> result;

    for (Value value : range) {
      if (value.type == type) {
        result.push_back(value);
      }
    }

    return result;
  }

  template <typename... Args>
  static void replace_values_helper(
      const std::unordered_map<Value, Value>& mapping, Args&&... args) {
    (replace_values_helper_helper(mapping, args), ...);
  }

 public:
  template <typename T>
    requires std::is_base_of_v<BaseInstruction, T>
  bool is_of_type() const {
    return typeid(*this) == typeid(T);
  }

  virtual std::string to_string() const = 0;

  virtual bool operator==(const BaseInstruction& other) const = 0;

  virtual std::vector<Value> filter_arguments(ValueType type) const = 0;

  virtual bool use_value(IR::Value value) const = 0;

  virtual bool has_return_value() const = 0;

  bool is_control_flow_instruction() const;

  virtual Value get_return_value() const = 0;

  virtual std::unique_ptr<BaseInstruction> clone() const = 0;
  virtual void accept(InstructionVisitor& visitor) const = 0;

  virtual void replace_values(
      const std::unordered_map<Value, Value>& mapping) = 0;

  virtual ~BaseInstruction() = default;
};

template <size_t NArgs, bool use_return>
struct Instruction : BaseInstruction {
  [[no_unique_address]] PossiblyEmptyStorage<!use_return, Value> return_value;
  std::array<Value, NArgs> arguments;

  Instruction(Value return_value, std::array<Value, NArgs> arguments)
    requires use_return
      : return_value(return_value), arguments(std::move(arguments)) {}

  Instruction(std::array<Value, NArgs> arguments)
    requires(!use_return)
      : arguments(std::move(arguments)) {}

  std::vector<Value> filter_arguments(ValueType type) const override {
    return filter_arguments_helper(arguments, type);
  }

  void replace_values(
      const std::unordered_map<Value, Value>& mapping) override {
    if constexpr (use_return) {
      return replace_values_helper(mapping, return_value.value, arguments);
    } else {
      return replace_values_helper(mapping, arguments);
    }
  }

  bool operator==(const BaseInstruction& other) const override {
    if (typeid(*this) != typeid(other)) {
      return false;
    }

    auto& casted = static_cast<const Instruction&>(other);
    return casted.arguments == arguments;
  }

  Value get_return_value() const override {
    if constexpr (use_return) {
      return return_value;
    } else {
      throw std::runtime_error(
          "Instruction has not return value but was asked to present it");
    }
  }

  bool use_value(Value value) const override {
    for (auto argument : arguments) {
      if (argument == value) {
        return true;
      }
    }

    return false;
  }

  bool has_return_value() const override { return use_return; }
};

template <bool use_return>
struct VariadicInstruction : BaseInstruction {
 protected:
  std::string join_arguments() const {
    return fmt::format(
        "{}", fmt::join(arguments | std::views::transform([](Value value) {
                          return value.to_string();
                        }),
                        ", "));
  }

 public:
  [[no_unique_address]] PossiblyEmptyStorage<!use_return, Value> return_value;
  std::vector<Value> arguments;

  VariadicInstruction(Value return_value, std::vector<Value> arguments)
    requires use_return
      : return_value(return_value), arguments(std::move(arguments)) {}

  VariadicInstruction(std::vector<Value> arguments)
    requires(!use_return)
      : arguments(std::move(arguments)) {}

  std::vector<Value> filter_arguments(ValueType type) const override {
    return filter_arguments_helper(arguments, type);
  }

  void replace_values(
      const std::unordered_map<Value, Value>& mapping) override {
    if constexpr (use_return) {
      return replace_values_helper(mapping, return_value.value, arguments);
    } else {
      return replace_values_helper(mapping, arguments);
    }
  }

  Value get_return_value() const override {
    if constexpr (use_return) {
      return return_value;
    } else {
      throw std::runtime_error(
          "Instruction has not return value but was asked to present it");
    }
  }

  bool operator==(const BaseInstruction& other) const override {
    if (typeid(*this) != typeid(other)) {
      return false;
    }

    auto& casted = static_cast<const VariadicInstruction&>(other);
    return casted.arguments == arguments;
  }

  bool use_value(Value value) const override {
    for (auto argument : arguments) {
      if (argument == value) {
        return true;
      }
    }

    return false;
  }

  bool has_return_value() const override { return use_return; }
};

struct FunctionCall final : VariadicInstruction<true> {
  std::string name;

  FunctionCall(Value return_value, const std::string& name,
               const std::vector<Value>& arguments = {})
      : VariadicInstruction(return_value, arguments), name(name) {}

  std::string to_string() const override {
    return fmt::format("{} = call {}({})", return_value, name,
                       join_arguments());
  }

  INSTRUCTION_MEMBERS()
};

struct Addition final : Instruction<2, true> {
  Addition(Value return_value, Value left, Value right)
      : Instruction(return_value, {left, right}) {}

  std::string to_string() const override {
    return fmt::format("{} = add {} {}", return_value, arguments[0],
                       arguments[1]);
  }

  INSTRUCTION_MEMBERS();
};

struct Subtraction final : Instruction<2, true> {
  Subtraction(Value return_value, Value left, Value right)
      : Instruction(return_value, {left, right}) {}

  std::string to_string() const override {
    return fmt::format("{} = sub {} {}", return_value, arguments[0],
                       arguments[1]);
  }

  INSTRUCTION_MEMBERS();
};

struct Move final : Instruction<1, true> {
  Move(Value return_value, Value source)
      : Instruction(return_value, {source}) {}

  std::string to_string() const override {
    return fmt::format("{} = {}", return_value, arguments[0]);
  }

  INSTRUCTION_MEMBERS();
};

struct Phi final : BaseInstruction {
  Value return_value;

  // index of BB in function BBs vector / temporary from this block
  std::vector<std::pair<BasicBlock*, Value>> parents;

  auto values_view() { return parents | std::views::elements<1>; }
  auto values_view() const { return parents | std::views::elements<1>; }

  std::string to_string() const override {
    auto arguments =
        parents |
        std::views::transform([](const std::pair<BasicBlock*, Value>& pair) {
          return fmt::format("({}, {})", pair.second,
                             static_cast<const void*>(pair.first));
        });

    return fmt::format("{} = phi [{}]", return_value,
                       fmt::join(arguments, ", "));
  }

  bool operator==(const BaseInstruction& other) const override {
    auto* other_phi = dynamic_cast<const Phi*>(&other);

    if (other_phi == nullptr) {
      return false;
    }

    return parents == other_phi->parents;
  }

  std::vector<Value> filter_arguments(ValueType type) const override {
    return filter_arguments_helper(values_view(), type);
  }

  void replace_values(
      const std::unordered_map<Value, Value>& mapping) override {
    replace_values_helper(mapping, return_value, values_view());
  }

  bool has_return_value() const override { return true; }

  bool use_value(Value value) const override { return false; }

  INSTRUCTION_MEMBERS()

  Value get_return_value() const override { return return_value; }
};

struct Return final : Instruction<1, false> {
  using Instruction::Instruction;

  explicit Return(Value value) : Instruction({value}) {}

  std::string to_string() const override {
    return fmt::format("return {}", arguments[0]);
  }

  INSTRUCTION_MEMBERS();
};

struct Branch final : Instruction<1, false> {
  // TODO: some kind of condition must be here
  explicit Branch(Value condition_value) : Instruction({condition_value}) {}

  std::string to_string() const override {
    return fmt::format("branch {} == 0?", arguments[0]);
  }

  INSTRUCTION_MEMBERS();
};

struct Jump final : Instruction<0, false> {
  explicit Jump() : Instruction({}) {}

  std::string to_string() const override { return fmt::format("jump"); }

  INSTRUCTION_MEMBERS();
};

struct Load final : Instruction<1, true> {
  Load(Value return_value, Value stack_index)
      : Instruction(return_value, {stack_index}) {
    if (stack_index.type != ValueType::STACK_INDEX) {
      throw std::runtime_error("Load instruction argument must be stack index");
    }
  }

  std::string to_string() const override {
    return fmt::format("{} = load {}", return_value, arguments[0]);
  }

  INSTRUCTION_MEMBERS();
};

struct Store final : Instruction<2, false> {
  Store(Value value, Value stack_index) : Instruction({value, stack_index}) {
    if (stack_index.type != ValueType::STACK_INDEX) {
      throw std::runtime_error(
          "Store instruction second argument must be valid stack index");
    }
  }

  std::string to_string() const override {
    return fmt::format("store {} into {}", arguments[0], arguments[1]);
  }

  INSTRUCTION_MEMBERS();
};

struct Select final : Instruction<3, true> {
  Select(Value result, Value condition_value, Value zero_branch,
         Value nonzero_branch)
      : Instruction(result, {condition_value, zero_branch, nonzero_branch}) {}

  std::string to_string() const override {
    return fmt::format("{} = {} == 0 ? {} : {}", return_value, arguments[0],
                       arguments[1], arguments[2]);
  }

  INSTRUCTION_MEMBERS();
};

inline bool BaseInstruction::is_control_flow_instruction() const {
  return is_of_type<Branch>() || is_of_type<Return>() || is_of_type<Jump>();
}

}  // namespace IR
