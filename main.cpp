#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lexis/LexicalAnalyzer.h"
#include "preprocessor/Preprocessor.h"
#include "syntax/AdditionSyntax.h"
#include "syntax/RecursiveFunctionsSyntax.h"
#include "syntax/buffalo/SyntaxTreeBuilder.h"

using namespace std;

// only for debug output
constexpr bool kShowDebugTrace = false;
size_t current_recursion_depth = 1;

vector<vector<string>> GetDefinitions(const string& program) {
  stringstream is(program);

  vector<vector<string>> result;
  string buffer;
  while (std::getline(is, buffer, ';')) {
    result.emplace_back();
    string current_string;

    for (char symbol : buffer) {
      if (symbol == '(' || symbol == ')' || symbol == ',' || symbol == '=') {
        if (!current_string.empty()) {
          result.back().push_back(std::move(current_string));
        }

        result.back().push_back(string{symbol});
        current_string.clear();
        continue;
      }

      current_string += symbol;
    }

    if (!current_string.empty()) {
      result.back().push_back(std::move(current_string));
    }
  }

  return std::move(result);
}

enum class NodeType { FunctionCall, Variable, Constant };

struct Node {
  string value;
  vector<Node*> children;
  Node* parent;
  NodeType type;
};

bool IsRecursive(const SyntaxNode& node) {
  return node.children.back()->type == SyntaxNodeType::RECURSION_PARAMETER;
}

struct FunctionDefinition {
  string name;
  // argument name/is used
  vector<pair<string, bool>> arguments;

  size_t arguments_count() const { return arguments.size(); }
};

struct Callable {
  virtual ~Callable() = default;
  virtual int operator()(const unordered_map<string, int>& arguments) = 0;
};

struct CompositeCallable : Callable {
  FunctionDefinition* function = nullptr;
  Callable* function_body = nullptr;
  vector<Callable*> argument_calls;

  int operator()(const unordered_map<string, int>& arguments) override {
    unordered_map<string, int> superfunction_arguments;
    superfunction_arguments.reserve(argument_calls.size());
    for (size_t i = 0; i < argument_calls.size(); ++i) {
      if (!function->arguments[i].second) {
        continue;
      }

      superfunction_arguments[function->arguments[i].first] =
          (*argument_calls[i])(arguments);
    }

    if constexpr (kShowDebugTrace) {
      cout << string(current_recursion_depth, '\t')
           << "Calling superfunction: " << function->name << endl;
      cout << string(current_recursion_depth, '\t') << " - arguments: ";

      for (auto [k, v] : superfunction_arguments) {
        cout << "(" << k << " " << v << ")"
             << " ";
      }

      cout << endl;
    }

    ++current_recursion_depth;
    int value = (*function_body)(superfunction_arguments);
    --current_recursion_depth;

    if constexpr (kShowDebugTrace) {
      cout << string(current_recursion_depth, '\t') << "result: " << value
           << endl;
    }

    return value;
  }
};

struct ProjectorCallable : Callable {
  string value;

  explicit ProjectorCallable(string value) : value(std::move(value)) {}

  int operator()(const unordered_map<string, int>& arguments) override {
    if constexpr (kShowDebugTrace) {
      cout << string(current_recursion_depth, '\t') << "Projector " << value
           << " returning: " << arguments.at(value) << endl;
    }

    return arguments.at(value);
  }
};

struct ConstantCallable : Callable {
  int value;

  explicit ConstantCallable(int value) : value(value) {}

  int operator()(const unordered_map<string, int>& arguments) override {
    if constexpr (kShowDebugTrace) {
      cout << string(current_recursion_depth, '\t')
           << "Constant returning: " << value << endl;
    }

    return value;
  }
};

struct SuccessorCallable : Callable {
  constexpr static char kParameterName[] = "x";
  int operator()(const unordered_map<string, int>& arguments) override {
    if constexpr (kShowDebugTrace) {
      cout << string(current_recursion_depth, '\t')
           << "Successor returning: " << (arguments.at(kParameterName) + 1)
           << endl;
    }

    return arguments.at(kParameterName) + 1;
  }
};

struct RecursiveCallable : Callable {
  FunctionDefinition* self_definition = nullptr;
  Callable* initial = nullptr;
  Callable* recursive = nullptr;

  bool recursive_argument_used = true;

  int operator()(const unordered_map<string, int>& arguments) override {
    if constexpr (kShowDebugTrace) {
      cout << string(current_recursion_depth, '\t')
           << "Recursive callable: " << self_definition->name << endl;
    }

    auto recursion_parameter = self_definition->arguments.back();

    if (arguments.at(recursion_parameter.first) == 0) {
      ++current_recursion_depth;
      int value = (*initial)(arguments);
      --current_recursion_depth;

      if constexpr (kShowDebugTrace) {
        cout << string(current_recursion_depth, '\t') << "Result: " << value
             << endl;
      }

      return value;
    }

    unordered_map<string, int> new_arguments = arguments;
    --new_arguments.at(recursion_parameter.first);

    if (recursive_argument_used) {
      int value = (*this)(new_arguments);
      new_arguments[self_definition->name] = value;
    }

    ++current_recursion_depth;
    int result = (*recursive)(new_arguments);
    --current_recursion_depth;

    if constexpr (kShowDebugTrace) {
      cout << string(current_recursion_depth, '\t') << "Result: " << result
           << endl;
    }

    return result;
  }
};

Callable* GetFunctionCallNode(
    unordered_map<string, pair<FunctionDefinition, Callable*>>& program,
    const SyntaxNode& value_node, unordered_set<string>& variables) {
  const string& value = value_node.value;

  if (value_node.type == SyntaxNodeType::CONSTANT) {
    return new ConstantCallable(std::stoi(value));
  }

  if (value_node.type == SyntaxNodeType::VARIABLE) {
    variables.insert(value);
    return new ProjectorCallable(value);
  }

  // node is function call
  CompositeCallable* result = new CompositeCallable;

  if (!program.contains(value_node.value)) {
    throw std::runtime_error("undefined function: " + value_node.value);
  }

  auto& function = program[value_node.value];
  result->function = &function.first;
  result->function_body = function.second;

  if (result->function->arguments_count() != value_node.children.size()) {
    throw std::runtime_error("function called with wrong count of arguments");
  }

  result->argument_calls.reserve(value_node.children.size());
  for (const auto& child : value_node.children) {
    result->argument_calls.push_back(
        GetFunctionCallNode(program, *child, variables));
  }

  return result;
}

pair<FunctionDefinition, Callable*> GetFunction(
    unordered_map<string, pair<FunctionDefinition, Callable*>>& program,
    const SyntaxNode& info_node, const SyntaxNode& value_node) {
  if (program.contains(info_node.value)) {
    throw std::runtime_error("usage before definition!");
  }

  unordered_set<string> variables;
  Callable* callable = GetFunctionCallNode(program, value_node, variables);

  FunctionDefinition definition;
  definition.name = info_node.value;
  size_t arguments_count = info_node.children.size();
  definition.arguments.resize(arguments_count);
  for (size_t i = 0; i < arguments_count; ++i) {
    string name = info_node.children[i]->value;
    definition.arguments[i] = {name, variables.contains(name)};
  }

  return {definition, callable};
}

void AddSystemFunctions(
    unordered_map<string, pair<FunctionDefinition, Callable*>>& program) {
  FunctionDefinition successor{"successor",
                               {{SuccessorCallable::kParameterName, true}}};
  SuccessorCallable* call_node = new SuccessorCallable;

  program["successor"] = {successor, call_node};
}

int main() {
  Preprocessor preprocessor;
  std::filesystem::path base_path =
      "/Users/mihailsimakov/Documents/Programs/CLionProjects/"
      "RecursiveFunctions/tests";

  preprocessor.add_file("arithmetics", base_path / "arithmetics.rec");

  preprocessor.set_main("arithmetics");

  string program_text = preprocessor.process();

  auto tokens = LexicalAnalyzer::get_tokens(program_text);

  Logger::disable_category(Logger::Category::SYNTAX);

  auto syntax_tree = SyntaxTreeBuilder::build(
      tokens, RecursiveFunctionsSyntax::GetSyntax(),
      RecursiveFunctionsSyntax::RuleIdentifiers::PROGRAM);

  unordered_map<string, pair<FunctionDefinition, Callable*>> program;
  AddSystemFunctions(program);

  for (const auto& assignment : syntax_tree->children) {
    const auto& info_node = *assignment->children.front();
    const auto& value_node = *assignment->children.back();

    string function_name = info_node.value;

    if (!IsRecursive(info_node)) {
      if (program.contains(function_name)) {
        throw std::runtime_error("redefinition!");
      }

      program.emplace(function_name,
                      std::move(GetFunction(program, info_node, value_node)));
    } else {
      bool is_first = info_node.children.back()->value == "0";

      if (is_first == program.contains(function_name)) {
        throw std::runtime_error(
            "Recursive function initial case must precede recursive case.");
      }

      size_t arguments_count = info_node.children.size();
      if (is_first) {
        RecursiveCallable* callable = new RecursiveCallable;
        unordered_set<string> variables;
        callable->initial = GetFunctionCallNode(program, value_node, variables);

        FunctionDefinition definition;
        definition.name = function_name;
        definition.arguments.resize(arguments_count);

        for (size_t i = 0; i < arguments_count - 1; ++i) {
          string argument_name = info_node.children[i]->value;
          definition.arguments[i] = {argument_name,
                                     variables.contains(argument_name)};
        }

        definition.arguments[arguments_count - 1] = {"", true};

        program[function_name] = {definition, callable};
        continue;
      }

      auto& function = program[function_name];

      RecursiveCallable* callable =
          static_cast<RecursiveCallable*>(function.second);

      callable->self_definition = &function.first;

      unordered_set<string> variables;
      callable->recursive = GetFunctionCallNode(program, value_node, variables);

      for (size_t i = 0; i < arguments_count; ++i) {
        string argument_name = info_node.children[i]->value;
        function.first.arguments[i] = {argument_name,
                                       function.first.arguments[i].second ||
                                           variables.contains(argument_name)};
      }

      callable->recursive_argument_used =
          variables.contains(function.first.name);
    }
  }

  cout << "prepared! Now executing..." << endl;

  size_t value_to_check = 37;
  cout << "is " << value_to_check << " prime?" << endl;

  Callable& prime_checker = *program["is_prime"].second;
  unordered_map<string, int> arguments = {{"number", value_to_check}};

  cout << (prime_checker(arguments) == 1 ? "YES, it's prime!"
                                         : "NO, it's not prime!");

  return 0;
}
