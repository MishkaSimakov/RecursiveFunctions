#include "lexis/LexicalAnalyzer.h"

#include <functional>

namespace Lexis {
std::string GetTokenDescription(const Token& token) {
  std::string result{token.type.toString()};

  if (!token.value().empty()) {
    result.append("{");
    result.append(token.value());
    result.append("}");
  }

  return result;
}

TokenType LexicalAnalyzer::get_symbol_affiliation(char symbol) {
  if (std::isdigit(symbol) != 0) {
    return TokenType::CONSTANT;
  }

  if (std::isalpha(symbol) != 0 || symbol == '_') {
    return TokenType::IDENTIFIER;
  }

  switch (symbol) {
    case '(':
      return TokenType::OPEN_PAREN;
    case ')':
      return TokenType::CLOSE_PAREN;
    case '{':
      return TokenType::OPEN_BRACE;
    case '}':
      return TokenType::CLOSE_BRACE;
    case '=':
      return TokenType::OP_EQUAL;
    case '+':
      return TokenType::OP_PLUS;
    case ',':
      return TokenType::COMMA;
    case ';':
      return TokenType::SEMICOLON;
    case ':':
      return TokenType::COLON;
    default:
      return TokenType::ERROR;
  }
}

TokenSolitaryMode LexicalAnalyzer::get_solitary_mode(TokenType type) {
  switch (type) {
    case TokenType::CONSTANT:
    case TokenType::IDENTIFIER:
      return TokenSolitaryMode::CONCATENATE;
    case TokenType::CLOSE_BRACE:
    case TokenType::CLOSE_PAREN:
      return TokenSolitaryMode::SEPARATE;
    default:
      return TokenSolitaryMode::EXPLODE;
  }
}

void LexicalAnalyzer::process_keywords(std::span<Token> tokens) {
  for (Token& token : tokens) {
    if (token.type == TokenType::IDENTIFIER &&
        token.value() == cImportKeyword) {
      token.type = TokenType::KW_IMPORT;
    }
  }
}

std::vector<Token> LexicalAnalyzer::get_tokens(
    const SourcesLoader::FileSource& source) {
  std::vector<Token> result;

  bool is_first = true;
  TokenType current_token_type = TokenType::ERROR;
  std::string::const_iterator current_range_begin;

  for (auto itr = source.content.cbegin(); itr != source.content.cend();
       ++itr) {
    char symbol = *itr;

    if (std::iswspace(symbol)) {
      continue;
    }

    TokenType affiliation = get_symbol_affiliation(symbol);

    if (affiliation == TokenType::ERROR) {
      throw std::runtime_error("error");
    }

    auto solitary_mode = get_solitary_mode(affiliation);

    if (is_first || affiliation != current_token_type ||
        solitary_mode == TokenSolitaryMode::SEPARATE) {
      if (!is_first) {
        result.emplace_back(
            current_token_type,
            SourceRange{source.index, current_range_begin, itr});
      }

      is_first = false;
      current_token_type = affiliation;
      current_range_begin = itr;
      continue;
    }

    if (solitary_mode == TokenSolitaryMode::EXPLODE) {
      // TODO: maybe make separate exception for this case
      throw std::runtime_error("Error");
    }
  }

  result.emplace_back(
      current_token_type,
      SourceRange{source.index, current_range_begin, source.content.end()});
  process_keywords(result);

  return result;
}
}  // namespace Lexis
