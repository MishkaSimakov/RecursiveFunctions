#pragma once

#include "sources/SourceLocation.h"
#include "utils/SmartEnum.h"

namespace Lexis {
ENUM(TokenType,
     // keywords
     KW_IMPORT, KW_RETURN, KW_EXPORT, KW_EXTERNAL, KW_IF, KW_ELSE, KW_WHILE,
     KW_CONTINUE, KW_BREAK, KW_TRUE, KW_FALSE, KW_NULLPTR,

     // types
     KW_INT, KW_BOOL, KW_CHAR,

     IDENTIFIER,  // variable or function name
     NUMBER,      // number literal
     STRING,      // "hello world"

     // operators
     EQUAL,       // operator =
     PLUS,        // operator +
     MINUS,       // operator -
     STAR,        // operator *
     AMPERSAND,   // operator &
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
