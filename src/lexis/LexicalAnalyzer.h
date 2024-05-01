#pragma once

#include <array>
#include <functional>
#include <string>
#include <vector>

#include "Exceptions.h"

namespace Lexing {
using std::string, std::array, std::function, std::vector;

enum class TokenType : size_t {
  IDENTIFIER,  // variable or function name
  CONSTANT,    // number
  LPAREN,      // left parenthesis
  RPAREN,      // right parenthesis
  OPERATOR,    // operator + or =
  SEMICOLON,   // semicolon for statements separation
  ASTERISK,    // asterisk for argmin function
  COMMA,       // comma for arguments separation
  ERROR,       // returned if some unexpected symbol appeared
};

enum class TokenSolitaryMode { CONCATENATE, SEPARATE, EXPLODE };

struct Token {
  TokenType type = TokenType::ERROR;
  string value;

  bool operator==(const Token&) const = default;
};

inline string GetTokenDescription(const Token& token) {
  switch (token.type) {
    case TokenType::IDENTIFIER:
      return "identifier(" + token.value + ")";
    case TokenType::CONSTANT:
      return "constant(" + token.value + ")";
    case TokenType::LPAREN:
      return "left parenthesis";
    case TokenType::RPAREN:
      return "right parenthesis";
    case TokenType::OPERATOR:
      return "operator(" + token.value + ")";
    case TokenType::SEMICOLON:
      return "semicolon";
    case TokenType::ASTERISK:
      return "asterisk";
    case TokenType::COMMA:
      return "comma";
    case TokenType::ERROR:
      return "error";
    default:
      return "something strange";
  }
}

class LexicalAnalyzer {
  static TokenType get_symbol_affiliation(char symbol) {
    if (std::isdigit(symbol) != 0) {
      return TokenType::CONSTANT;
    }

    if (std::isalpha(symbol) != 0 || symbol == '_') {
      return TokenType::IDENTIFIER;
    }

    switch (symbol) {
      case '(':
        return TokenType::LPAREN;
      case ')':
        return TokenType::RPAREN;
      case '=':
      case '+':
        return TokenType::OPERATOR;
      case '*':
        return TokenType::ASTERISK;
      case ',':
        return TokenType::COMMA;
      case ';':
        return TokenType::SEMICOLON;
      default:
        return TokenType::ERROR;
    }
  }

  static TokenSolitaryMode get_solitary_mode(TokenType type) {
    switch (type) {
      case TokenType::CONSTANT:
      case TokenType::IDENTIFIER:
        return TokenSolitaryMode::CONCATENATE;
      case TokenType::RPAREN:
        return TokenSolitaryMode::SEPARATE;
      default:
        return TokenSolitaryMode::EXPLODE;
    }
  }

 public:
  static vector<Token> get_tokens(const string& program) {
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
};
}  // namespace Lexing
