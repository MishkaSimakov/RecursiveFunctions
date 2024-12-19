#include "lexis/LexicalAnalyzer.h"

#include <functional>

using Lexing::LexicalAnalyzer, Lexing::Token;
using std::string, std::array, std::function, std::vector;

void LexicalAnalyzer::process_keywords(std::span<Token> tokens) {
  for (Token& token : tokens) {
    if (token.type == TokenType::IDENTIFIER && token.value == cExternKeyword) {
      token.type = TokenType::KW_EXTERN;
    }
  }
}

vector<Token> LexicalAnalyzer::get_tokens(const string& program) {
  vector<Token> result;

  bool is_first = true;
  Token current_token;

  for (size_t i = 0; i < program.size(); ++i) {
    char symbol = program[i];
    TokenType affiliation = get_symbol_affiliation(symbol);

    if (affiliation == TokenType::ERROR) {
      throw std::runtime_error("error");
    }

    auto solitary_mode = get_solitary_mode(affiliation);

    if (is_first || affiliation != current_token.type ||
        solitary_mode == TokenSolitaryMode::SEPARATE) {
      if (!is_first) {
        result.push_back(current_token);
      }

      is_first = false;
      current_token.type = affiliation;
      current_token.value = symbol;
      continue;
    }

    if (solitary_mode == TokenSolitaryMode::EXPLODE) {
      // TODO: maybe make separate exception for this case
      throw std::runtime_error("Error");
    }

    current_token.value += symbol;
  }

  result.push_back(current_token);
  process_keywords(result);

  return result;
}
