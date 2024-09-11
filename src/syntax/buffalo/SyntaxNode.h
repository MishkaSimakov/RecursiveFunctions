#pragma once

#include <memory>
#include <string>
#include <vector>

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

  bool operator==(const SyntaxNode& other) const {
    if (type != other.type || value != other.value || children.size() != other.children.size()) {
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
