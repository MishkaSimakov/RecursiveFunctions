#pragma once

#include "sources/SourceLocation.h"
#include "utils/SmartEnum.h"

namespace Lexis {
ENUM(TokenType,
     // keywords
     KW_BREAK, KW_CONTINUE, KW_ELSE, KW_IMPORT, KW_EXPORT, KW_EXTERN, KW_IF,
     KW_NAMESPACE, KW_RETURN, KW_USING, KW_WHILE,

     // literals
     KW_NULLPTR, KW_FALSE, KW_TRUE,

     // types
     KW_U64, KW_I64, KW_F64, KW_BOOL, KW_CHAR, KW_VOID,
     // keywords end

     IDENTIFIER,  // variable or function name
     NUMBER,      // number literal
     STRING,      // "hello world"

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
