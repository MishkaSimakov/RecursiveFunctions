#pragma once

#include "sources/SourceLocation.h"
#include "utils/SmartEnum.h"

namespace Lexis {
ENUM(TokenType,
     IDENTIFIER,  // variable or function name

     // keywords
     // flow control
     KW_BREAK, KW_CONTINUE, KW_ELSE, KW_IF, KW_RETURN, KW_WHILE,

     //
     KW_IMPORT, KW_EXPORT, KW_EXTERN, KW_NAMESPACE, KW_TYPE,

     // literals
     KW_NULLPTR, KW_FALSE, KW_TRUE,

     // types
     // signed int
     KW_I8, KW_I16, KW_I32, KW_I64,

     // unsigned int
     KW_U8, KW_U16, KW_U32, KW_U64,

     // char
     KW_C8,

     // bool
     KW_B8,
     // keywords end

     NUMBER,  // number literal
     STRING,  // "hello world"

     // operators
     EQUAL,       // =
     PLUS,        // +
     MINUS,       // -
     STAR,        // *
     PERCENT,     // %
     AMPERSAND,   // &
     LESS,        // <
     GREATER,     // >
     LESS_EQ,     // <=
     GREATER_EQ,  // >=
     EQUALEQUAL,  // ==
     NOTEQUAL,    // !=
     ARROW,       // ->
     PLUSPLUS,    // ++
     MINUSMINUS,  // --

     COLONCOLON,  // ::

     // special symbols
     OPEN_PAREN,   // (
     CLOSE_PAREN,  // )
     OPEN_BRACE,   // {
     CLOSE_BRACE,  // }
     SEMICOLON,    // ;
     COLON,        // :
     COMMA,        // ,
     DOT,          // .

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
