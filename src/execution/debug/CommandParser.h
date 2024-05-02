#pragma once

#include <concepts>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using std::string, std::map, std::vector;

template <typename T>
struct ArgumentParser;

template <>
struct ArgumentParser<size_t> {
  size_t operator()(const string& argument) const {
    int value = std::stoi(argument);

    if (value < 0) {
      throw std::invalid_argument("Could not parse argument.");
    }

    return static_cast<size_t>(value);
  }
};

template <>
struct ArgumentParser<string> {
  string operator()(const string& argument) const { return argument; }
};

class CommandParser {
  map<string, std::function<void(const vector<string>&)>> commands_;

  template <typename... Args, typename F, size_t... I>
  static void invoke(std::index_sequence<I...>, const F& func,
                     const vector<string>& arguments) {
    func(ArgumentParser<Args>()(arguments[I])...);
  }

 public:
  template <typename... Args, std::invocable<Args...> F>
    requires(std::is_same_v<Args, decltype(ArgumentParser<Args>()(
                                      std::declval<string>()))> &&
             ...)
  void add_command(string name, F&& func) {
    commands_.emplace(
        name, [func = std::move(func)](const vector<string>& arguments) {
          if (arguments.size() != sizeof...(Args)) {
            throw std::runtime_error("Wrong number of arguments");
          }

          invoke<Args...>(std::index_sequence_for<Args...>(), func, arguments);
        });
  }

  void parse(string command) const {
    vector<string> arguments;
    size_t pos;

    while ((pos = command.find(" ")) != std::string::npos) {
      std::cout << "here" << std::endl;
      string value = command.substr(0, pos);
      command.erase(0, pos + 1);
    }

    std::print(std::cout, "arguments count {}", arguments.size());
    for (auto c: arguments) {
      cout << "argument " << c << endl;
    }

    string command_name = arguments.front();
    arguments.erase(arguments.begin());

    auto command_itr = commands_.find(command_name);
    if (command_itr == commands_.end()) {
      throw std::runtime_error("Unknown command type");
    }

    command_itr->second(arguments);
  }
};