#pragma once

#include <array>
#include <ranges>
#include <string>
#include <vector>

#include "sources/SourcesLoader.h"
#include "utils/SmartEnum.h"

namespace Lexis {
ENUM(TokenType,
     IDENTIFIER,  // variable or function name
     NUMBER,    // number
     STRING,      // "hello world"

     // operators
     EQUAL,       // operator =
     PLUS,        // operator +
     LESS,        // <
     GREATER,     // >
     LESS_EQ,     // <=
     GREATER_EQ,  // >=

     // keywords
     KW_IMPORT,  // import keyword

     // special symbols
     OPEN_PAREN,   // (
     CLOSE_PAREN,  // )
     OPEN_BRACE,   // {
     CLOSE_BRACE,  // }
     SEMICOLON,    // ;
     COLON,        // :
     COMMA,        // ,

     ERROR,  // returned if some unexpected symbol appeared

     END  // reserved for sequence end in grammar parsing
);

enum class TokenSolitaryMode { CONCATENATE, SEPARATE, EXPLODE };

struct Token {
  TokenType type = TokenType::ERROR;
  const SourceRange source_range{};

  std::string_view value() const { return source_range.value(); }

  bool operator==(const Token& other) const {
    return type == other.type && value() == other.value();
  }
};

std::string GetTokenDescription(const Token& token);

class LexicalAnalyzer {
  static constexpr auto cImportKeyword = "import";

  static TokenType get_symbol_affiliation(char symbol);
  static TokenSolitaryMode get_solitary_mode(TokenType type);
  static void process_keywords(std::span<Token> tokens);

 public:
  static std::vector<Token> get_tokens(const SourcesLoader::FileSource& source);
};
}  // namespace Lexis
