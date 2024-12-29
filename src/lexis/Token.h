#pragma once

#include "sources/SourceLocation.h"
#include "utils/SmartEnum.h"

namespace Lexis {
ENUM(TokenType,
     // keywords
     KW_IMPORT,  // import keyword
     KW_INT,     // int keyword
     KW_RETURN,  // return keyword

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

  SourceRange source_range;
};
}  // namespace Lexis
