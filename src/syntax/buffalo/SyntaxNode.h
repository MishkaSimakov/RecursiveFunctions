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
  ASSIGNMENT,
  RECURSION_PARAMETER
};

struct SyntaxNode {
  SyntaxNodeType type;
  string value;
  vector<unique_ptr<SyntaxNode>> children;

  explicit SyntaxNode(SyntaxNodeType type, string value = "")
      : type(type), value(std::move(value)) {}
};
