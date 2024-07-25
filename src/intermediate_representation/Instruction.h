#pragma once

#include <fmt/base.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <ranges>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include "Temporary.h"

namespace IR {
struct BasicBlock;

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
    requires std::is_base_of_v<Instruction, T>
  bool is_of_type() const {
    return typeid(*this) == typeid(T);
  }

 protected:
  template <typename... Args>
    requires(
        (std::is_same_v<std::decay_t<Args>, Temporary> ||
         std::is_same_v<std::decay_t<Args>, TemporaryOrConstant> ||
         std::is_same_v<std::decay_t<Args>, std::vector<TemporaryOrConstant>> ||
         std::is_same_v<std::decay_t<Args>, std::vector<Temporary>>) &&
        ...)
  static std::vector<Temporary> filter_temporaries(Args&&... args) {
    std::vector<Temporary> result;

    (filter_temporaries_inserter(result, std::forward<decltype(args)>(args)),
     ...);
    return result;
  }

 public:
  Temporary result_destination;

  virtual std::string to_string() const = 0;

  bool operator==(const Instruction& other) const {
    return typeid(*this) == typeid(other) && equal(other);
  }

  virtual std::vector<Temporary> get_temporaries_in_arguments() const = 0;

  bool has_return_value() const;

  virtual ~Instruction() = default;
};

struct FunctionCall final : Instruction {
  std::string name;
  std::vector<TemporaryOrConstant> arguments;

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

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const FunctionCall&>(instruction);

    return name == other.name && arguments == other.arguments;
  }
};

struct Addition final : Instruction {
  Temporary left;
  TemporaryOrConstant right;

  std::string to_string() const override {
    return fmt::format("{} = add {} {}", result_destination, left, right);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return filter_temporaries(left, right);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Addition&>(instruction);

    return left == other.left && right == other.right;
  }
};

struct Subtraction final : Instruction {
  Temporary left;
  TemporaryOrConstant right;

  std::string to_string() const override {
    return fmt::format("{} = sub {} {}", result_destination, left, right);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return filter_temporaries(left, right);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Subtraction&>(instruction);

    return left == other.left && right == other.right;
  }
};

struct Move final : Instruction {
  TemporaryOrConstant source;

  std::string to_string() const override {
    return fmt::format("{} = {}", result_destination, source);
  }

  std::vector<Temporary> get_temporaries_in_arguments() const override {
    return filter_temporaries(source);
  }

 private:
  bool equal(const Instruction& instruction) const override {
    auto& other = static_cast<const Move&>(instruction);

    return source == other.source;
  }
};

struct Phi final : Instruction {
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
