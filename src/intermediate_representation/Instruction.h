#pragma once

#include <fmt/base.h>
#include <fmt/ranges.h>

#include <ranges>
#include <string>
#include <vector>

namespace IR {
struct Temporary {
  size_t index;

  std::string to_string() const { return "%" + std::to_string(index); }
};

struct TemporaryOrConstant {
  ssize_t index_or_value{0};
  bool is_constant{true};

  TemporaryOrConstant() = default;

  TemporaryOrConstant(ssize_t index_or_value, bool is_constant)
      : index_or_value(index_or_value), is_constant(is_constant) {}

  TemporaryOrConstant(Temporary temporary)
      : index_or_value(temporary.index), is_constant(false) {}

  size_t index() const {
    if (is_constant) {
      throw std::runtime_error("In IR constant used as temporary");
    }

    return index_or_value;
  }

  ssize_t value() const {
    if (!is_constant) {
      throw std::runtime_error("In IR temporary used as constant");
    }

    return index_or_value;
  }

  static TemporaryOrConstant temporary(size_t index) {
    return {static_cast<ssize_t>(index), false};
  }

  static TemporaryOrConstant constant(ssize_t value) { return {value, true}; }

  std::string to_string() const {
    if (is_constant) {
      return std::to_string(index_or_value);
    }

    return "%" + std::to_string(index_or_value);
  }
};
}  // namespace IR

template <>
struct fmt::formatter<IR::TemporaryOrConstant> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const IR::TemporaryOrConstant& value, FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.to_string());
  }
};

template <>
struct fmt::formatter<IR::Temporary> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const IR::Temporary& value, FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.to_string());
  }
};

namespace IR {
struct Instruction {
  Temporary result_destination;

  virtual std::string to_string() const = 0;

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
};

struct Addition final : Instruction {
  Temporary left;
  TemporaryOrConstant right;

  std::string to_string() const override {
    return fmt::format("{} = add {} {}", result_destination, left, right);
  }
};

struct Subtraction final : Instruction {
  Temporary left;
  TemporaryOrConstant right;

  std::string to_string() const override {
    return fmt::format("{} = sub {} {}", result_destination, left, right);
  }
};

struct Move final : Instruction {
  TemporaryOrConstant source;

  std::string to_string() const override {
    return fmt::format("{} = {}", result_destination, source);
  }
};

struct Phi final : Instruction {
  std::vector<TemporaryOrConstant> values;

  std::string to_string() const override {
    return fmt::format(
        "{} = phi [{}]", result_destination,
        fmt::join(values | std::views::transform([](TemporaryOrConstant value) {
                    return value.to_string();
                  }),
                  ", "));
  }
};
}  // namespace IR
