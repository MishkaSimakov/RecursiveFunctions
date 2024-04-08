#ifndef SYNTAXNODE_H
#define SYNTAXNODE_H

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

  // SyntaxNode(const SyntaxNode& other) {
  //   std::cout << "copied" << std::endl;
  //
  //   type = other.type;
  //   value = other.value;
  //   children = other.children;
  // }
};

#endif  // SYNTAXNODE_H
