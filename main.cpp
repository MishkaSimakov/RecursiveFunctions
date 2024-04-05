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

bool IsUnsignedNumber(const string& value) {
  return std::ranges::all_of(
      value.begin(), value.end(),
      [](char value) -> bool { return std::isdigit(value) != 0; });
}

pair<Node*, Node*> ParseFunction(const vector<string>& definition) {
  // create temp node
  Node temp_node;
  Node* current_node = &temp_node;

  for (const string& token : definition) {
    if (token == "=") {
      continue;
    }

    if (token == "(") {
      current_node = current_node->children.back();
      current_node->type = NodeType::FunctionCall;
      continue;
    }

    if (token == ")") {
      current_node = current_node->parent;
      continue;
    }

    if (token == ",") {
      continue;
    }

    Node* node = new Node;
    node->value = token;
    current_node->children.push_back(node);
    node->parent = current_node;
    node->type =
        IsUnsignedNumber(token) ? NodeType::Constant : NodeType::Variable;
  }

  Node* left_child = temp_node.children.front();
  left_child->parent = nullptr;

  Node* right_child = temp_node.children.back();
  right_child->parent = nullptr;

  return {left_child, right_child};
}

bool IsRecursive(Node* function) {
  string last_argument = function->children.back()->value;
  return last_argument == "0" || last_argument.ends_with("+1");
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
    Node* node, unordered_set<string>& variables) {
  if (node->type == NodeType::Constant) {
    return new ConstantCallable(std::stoi(node->value));
  }

  if (node->type == NodeType::Variable) {
    variables.insert(node->value);
    return new ProjectorCallable(node->value);
  }

  // node is function call
  CompositeCallable* result = new CompositeCallable;

  if (!program.contains(node->value)) {
    throw std::runtime_error("undefined function: " + node->value);
  }

  auto& function = program[node->value];
  result->function = &function.first;
  result->function_body = function.second;

  if (result->function->arguments_count() != node->children.size()) {
    throw std::runtime_error("function called with wrong count of arguments");
  }

  result->argument_calls.reserve(node->children.size());
  for (const auto& child : node->children) {
    result->argument_calls.push_back(
        GetFunctionCallNode(program, child, variables));
  }

  return result;
}

pair<FunctionDefinition, Callable*> GetFunction(
    unordered_map<string, pair<FunctionDefinition, Callable*>>& program,
    Node* definition_node, Node* value_node) {
  if (program.contains(definition_node->value)) {
    throw std::runtime_error("usage before definition!");
  }

  unordered_set<string> variables;
  Callable* callable = GetFunctionCallNode(program, value_node, variables);

  FunctionDefinition definition;
  definition.name = definition_node->value;
  size_t arguments_count = definition_node->children.size();
  definition.arguments.resize(arguments_count);
  for (size_t i = 0; i < arguments_count; ++i) {
    string name = definition_node->children[i]->value;
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

// int main() {
//   string filename =
//       "/Users/mihailsimakov/Documents/Programs/CLionProjects/"
//       "RecursiveFunctions/tests/arithmetics.rec";
//
//   auto lines = GetDefinitions(filename);
//
//   unordered_map<string, pair<FunctionDefinition, Callable*>> program;
//
//   AddSystemFunctions(program);
//
//   for (const auto& line : lines) {
//     auto [name, value] = ParseFunction(line);
//
//     string function_name = name->value;
//
//     if (!IsRecursive(name)) {
//       if (program.contains(function_name)) {
//         throw std::runtime_error("redefinition!");
//       }
//
//       program.emplace(function_name,
//                       std::move(GetFunction(program, name, value)));
//     } else {
//       bool is_first = name->children.back()->value == "0";
//
//       if (is_first == program.contains(name->value)) {
//         throw std::runtime_error(
//             "Recursive function initial case must precede recursive case.");
//       }
//
//       size_t arguments_count = name->children.size();
//       if (is_first) {
//         RecursiveCallable* callable = new RecursiveCallable;
//         unordered_set<string> variables;
//         callable->initial = GetFunctionCallNode(program, value, variables);
//
//         FunctionDefinition definition;
//         definition.name = name->value;
//         definition.arguments.resize(arguments_count);
//
//         for (size_t i = 0; i < arguments_count - 1; ++i) {
//           string argument_name = name->children[i]->value;
//           definition.arguments[i] = {argument_name,
//                                      variables.contains(argument_name)};
//         }
//
//         definition.arguments[arguments_count - 1] = {"", true};
//
//         program[name->value] = {definition, callable};
//         continue;
//       }
//
//       auto& function = program[name->value];
//
//       RecursiveCallable* callable =
//           static_cast<RecursiveCallable*>(function.second);
//
//       callable->self_definition = &function.first;
//
//       unordered_set<string> variables;
//       callable->recursive = GetFunctionCallNode(program, value, variables);
//
//       for (size_t i = 0; i < arguments_count; ++i) {
//         string argument_name = name->children[i]->value;
//         function.first.arguments[i] = {argument_name,
//                                        function.first.arguments[i].second ||
//                                            variables.contains(argument_name)};
//       }
//
//       auto& back = function.first.arguments.back().first;
//       back.pop_back();
//       back.pop_back();
//
//       callable->recursive_argument_used =
//           variables.contains(function.first.name);
//     }
//   }
//
//   cout << "prepared! Now executing..." << endl;
//
//   cout << "is 37 prime?" << endl;
//
//   Callable& prime_checker = *program["is_prime"].second;
//   unordered_map<string, int> arguments = {{"number", 5}};
//
//   cout << (prime_checker(arguments) == 1 ? "YES, it's prime!"
//                                          : "NO, some cringe has happened");
//
//   return 0;
// }

int main() {
  Preprocessor preprocessor;
  std::filesystem::path base_path =
      "/Users/mihailsimakov/Documents/Programs/CLionProjects/"
      "RecursiveFunctions/tests";

  preprocessor.add_file("arithmetics", base_path / "arithmetics.rec");
  preprocessor.add_file("secret", base_path / "secret.rec");

  preprocessor.set_main("secret");

  string program = preprocessor.process();
  std::cout << program << std::endl;

  auto tokens = LexicalAnalyzer::get_tokens(program);

  for (auto& token : tokens) {
    cout << static_cast<size_t>(token.type) << " " << token.value << endl;
  }
}