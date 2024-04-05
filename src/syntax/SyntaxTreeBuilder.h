#ifndef SYNTAXTREEBUILDER_H
#define SYNTAXTREEBUILDER_H

#include <iostream>
#include <string>
#include <vector>

#include "lexis/LexicalAnalyzer.h"

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
  static SyntaxNode build(const vector<Token>& program) {

  }
};

#endif  // SYNTAXTREEBUILDER_H
