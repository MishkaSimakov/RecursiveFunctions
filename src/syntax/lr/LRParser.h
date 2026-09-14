#pragma once

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Action.h"
#include "compilation/ModuleContext.h"
#include "lexis/LexicalAnalyzer.h"

namespace Syntax {
class ParserException final : public std::runtime_error {
  std::vector<std::pair<SourceRange, std::string>> errors_;

 public:
  explicit ParserException(
      std::vector<std::pair<SourceRange, std::string>> errors)
      : std::runtime_error("Parser error."), errors_(std::move(errors)) {}

  const auto& get_errors() const { return errors_; }
};

class LRParser {
  Action get_action(size_t state, Lexis::TokenType token) const;
  size_t get_goto(size_t state, NonTerminal non_terminal) const;

 public:
  void parse(Lexis::LexicalAnalyzer& lexical_analyzer,
             Front::ModuleContext& context, SourceView source) const;
};
}  // namespace Syntax
