#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "lexis/LexicalAnalyzer.h"

using std::string, std::vector, std::unique_ptr;

enum class SyntaxNodeType {
  ROOT,
  VARIABLE,
  CONSTANT,
  ASTERISK,
  FUNCTION,
  EXTERN_SPECIFIER,
  ASSIGNMENT,
  RECURSION_PARAMETER,
};

struct SyntaxNode {
  SyntaxNodeType type;
  string value;
  vector<unique_ptr<SyntaxNode>> children;

  explicit SyntaxNode(SyntaxNodeType type, string value = "")
      : type(type), value(std::move(value)) {}

  explicit SyntaxNode(const Lexis::Token& token)
      : type(SyntaxNodeType::ROOT), value(token.value) {}

  bool operator==(const SyntaxNode& other) const {
    if (type != other.type || value != other.value ||
        children.size() != other.children.size()) {
      return false;
    }

    for (size_t i = 0; i < children.size(); ++i) {
      if (*children[i] != *other.children[i]) {
        return false;
      }
    }

    return true;
  }
};

inline void PrintSyntaxTreeRecursive(const std::string& prefix,
                                     const SyntaxNode& node, bool is_last) {
  std::cout << prefix;

  std::cout << (is_last ? "└── " : "├── ");

  // print the value of the node
  string result;
  switch (node.type) {
    case SyntaxNodeType::ROOT:
      result = "Root";
      break;
    case SyntaxNodeType::VARIABLE:
      result = "Var{" + node.value + "}";
      break;
    case SyntaxNodeType::CONSTANT:
      result = "Num{" + node.value + "}";
      break;
    case SyntaxNodeType::ASTERISK:
      result = "*";
      break;
    case SyntaxNodeType::FUNCTION:
      result = "Func{" + node.value + "}";
      break;
    case SyntaxNodeType::ASSIGNMENT:
      result = "=";
      break;
    case SyntaxNodeType::RECURSION_PARAMETER:
      result = "Rec{" + node.value + "}";
      break;
    case SyntaxNodeType::EXTERN_SPECIFIER:
      result = "Extern";
      break;
  }
  std::cout << result << std::endl;

  // enter the next tree level - left and right branch
  for (size_t i = 0; i < node.children.size(); ++i) {
    PrintSyntaxTreeRecursive(prefix + (is_last ? "    " : "│   "),
                             *node.children[i], i == node.children.size() - 1);
  }
}