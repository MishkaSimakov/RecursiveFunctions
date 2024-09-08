#include "LexicalAnalysisStage.h"

std::vector<Lexing::Token> LexicalAnalysisStage::apply(const std::string& program) {
  return Lexing::LexicalAnalyzer::get_tokens(program);
}
