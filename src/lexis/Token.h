#pragma once

#include "utils/SmartEnum.h"

namespace Lexis {
ENUM(TokenType,
     IDENTIFIER,  // variable or function name
     NUMBER,      // number
     STRING,      // "hello world"

     // operators
     EQUAL,       // operator =
     PLUS,        // operator +
     LESS,        // <
     GREATER,     // >
     LESS_EQ,     // <=
     GREATER_EQ,  // >=

     ARROW,  // ->

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

     // special values
     WHITESPACE,  // used only inside lexical analyzer
     COMMENT,     // used only inside lexical analyzer
     ERROR,       // returned if some unexpected symbol appeared

     END  // reserved for sequence end in grammar parsing
);

struct Token {
  TokenType type = TokenType::ERROR;
  std::string value;

  bool operator==(const Token& other) const = default;
};
}