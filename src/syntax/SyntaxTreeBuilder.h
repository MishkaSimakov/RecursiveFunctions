#ifndef SYNTAXTREEBUILDER_H
#define SYNTAXTREEBUILDER_H

#include <iostream>
#include <string>
#include <vector>

using std::string, std::vector, std::cout, std::endl;

enum class SyntaxNodeType { VARIABLE, ASTERISK, FUNCTION, ASSIGNMENT };

struct SyntaxNode {
  SyntaxNodeType type;
  string value;
  vector<SyntaxNode> children;

  SyntaxNode(const SyntaxNode& other) {
    std::cout << "copied" << std::endl;

    type = other.type;
    value = other.value;
    children = other.children;
  }
};

class SyntaxTreeBuilder {
 public:
  SyntaxNode build(const string& program) {

  }
};

#endif  // SYNTAXTREEBUILDER_H
