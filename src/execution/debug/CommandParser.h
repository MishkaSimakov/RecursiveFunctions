#pragma once

#include <concepts>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using std::string, std::map, std::vector, std::cout, std::cin, std::endl;

static int stoi_with_custom_exceptions(const string& value) {
  try {
    return std::stoi(value);
  } catch (std::invalid_argument) {
    throw std::runtime_error("Could not parse argument.");
  } catch (std::out_of_range) {
    throw std::runtime_error("Given value is too big.");
  }
}

template <typename T>
struct ArgumentParser;

template <>
struct ArgumentParser<size_t> {
  size_t operator()(const string& argument) const {
    int value = stoi_with_custom_exceptions(argument);

    if (value < 0) {
      throw std::runtime_error("Value must be non-negative.");
    }

    return static_cast<size_t>(value);
  }
};

struct StackSlice {
  enum class StackType { CALL_STACK, ARGUMENTS_STACK, CALCULATIONS_STACK };

  static std::optional<StackType> get_stack_type_from_string(
      const string& value) {
    if (value == "call") {
      return StackType::CALL_STACK;
    }
    if (value == "args") {
      return StackType::ARGUMENTS_STACK;
    }
    if (value == "calc") {
      return StackType::CALCULATIONS_STACK;
    }

    return {};
  }

  StackSlice(StackType stack, std::optional<std::pair<size_t, size_t>> range)
      : stack(stack), range(range) {}

  StackType stack;
  std::optional<std::pair<size_t, size_t>> range;
};

template <>
struct ArgumentParser<StackSlice> {
  static const std::regex kRangefullStackSliceRegex;
  static const std::regex kRanglessStackSliceRegex;

  StackSlice operator()(const string& argument) const {
    std::smatch matches;
    std::optional<std::pair<size_t, size_t>> range;

    if (std::regex_match(argument, matches, kRangefullStackSliceRegex)) {
      int from = stoi_with_custom_exceptions(matches[2]);
      int to = stoi_with_custom_exceptions(matches[3]);

      if (from < 0 || to < 0) {
        throw std::runtime_error(
            "Range boundaries in stack slice must be non-negative");
      }
      if (to < from) {
        throw std::runtime_error(
            "Left border of range must precede right border");
      }

      range = {from, to};
    } else if (!std::regex_match(argument, matches, kRanglessStackSliceRegex)) {
      throw std::runtime_error("Wrong print command argument format.");
    }

    auto stack_type = StackSlice::get_stack_type_from_string(matches[1]);

    if (!stack_type) {
      throw std::runtime_error(
          "Wrong stack type value. Available values: call, args and calc");
    }

    return StackSlice(*stack_type, range);
  }
};

inline const std::regex ArgumentParser<StackSlice>::kRangefullStackSliceRegex{
    R"((.*)\[(-?\d+):(-?\d+)\])"};
inline const std::regex ArgumentParser<StackSlice>::kRanglessStackSliceRegex{
    R"((.*))"};

template <>
struct ArgumentParser<string> {
  string operator()(const string& argument) const { return argument; }
};

template <typename T>
class CommandParser {
  T* object_ptr_;
  map<string, std::function<void(const vector<string>&)>> commands_;

  template <typename... Args, typename F, size_t... I>
  void invoke(std::index_sequence<I...>, const F& func,
              const vector<string>& arguments) {
    (object_ptr_->*func)(ArgumentParser<Args>()(arguments[I])...);
  }

 public:
  explicit CommandParser(T* object_ptr) : object_ptr_(object_ptr) {}

  template <typename... Args, typename F>
    requires(std::is_same_v<Args, decltype(ArgumentParser<Args>()(
                                      std::declval<string>()))> &&
             ...)
  void add_command(string name, F&& func) {
    commands_.emplace(
        name, [this, func = std::move(func)](const vector<string>& arguments) {
          if (arguments.size() != sizeof...(Args)) {
            throw std::runtime_error("Wrong number of arguments");
          }

          invoke<Args...>(std::index_sequence_for<Args...>(), func, arguments);
        });
  }

  void parse(string command) const {
    vector<string> arguments;
    size_t pos;

    do {
      pos = command.find(" ");

      string value = command.substr(0, pos);
      command.erase(0, pos != string::npos ? pos + 1 : string::npos);

      arguments.push_back(value);
    } while (pos != string::npos);

    string command_name = arguments.front();
    arguments.erase(arguments.begin());

    auto command_itr = commands_.find(command_name);
    if (command_itr == commands_.end()) {
      throw std::runtime_error("Unknown command type");
    }

    command_itr->second(arguments);
  }
};
