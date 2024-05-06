#include "lexis/LexicalAnalyzer.h"

using Lexing::LexicalAnalyzer, Lexing::Token;
using std::string, std::array, std::function, std::vector;

vector<Token> LexicalAnalyzer::get_tokens(const string& program) {
  vector<Token> result;

  bool is_first = true;
  Token current_token;

  for (size_t i = 0; i < program.size(); ++i) {
    char symbol = program[i];
    TokenType affiliation = get_symbol_affiliation(symbol);

    if (affiliation == TokenType::ERROR) {
      throw UnexpectedSymbolException(program, i);
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
      throw UnexpectedSymbolException(program, i);
    }

    current_token.value += symbol;
  }

  result.push_back(current_token);

  return result;
}