#pragma once

#include <string>

#include "lexis/LexicalAnalyzer.h"

class LexicalAnalysisStage {
 public:
  using input = const std::string&;
  using output = std::vector<Lexing::Token>;
  constexpr static auto name = "lexical_analysis";

  std::vector<Lexing::Token> apply(const std::string& program);
};
