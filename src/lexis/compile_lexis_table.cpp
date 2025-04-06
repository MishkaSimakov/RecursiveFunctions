#include <fmt/base.h>

#include "lexis/Token.h"
#include "table/LexicalAutomatonGenerator.h"
#include "utils/Constants.h"

int main() {
  using namespace Lexis;

  LexicalAutomatonGenerator generator;

  generator["letter"] = "[a-zA-Z]";
  generator["digit"] = "[0-9]";
  generator["space"] = "[ \t\r\n\v\f]";
  generator["comment_symbol"] = "[^\n]";
  generator["s_char"] = "[^\"\n]";

  // IDENTIFIER token can be overriden. Therefore, we mark this token as "weak".
  // All warnings from collisions with this token will be ignored
  // (same as weak linkage)
  generator[TokenType::IDENTIFIER] = "[a-zA-Z_]([a-zA-Z0-9_])*";
  generator.mark_token_as_weak(TokenType::IDENTIFIER);

  // keywords
  // keywords
  generator[TokenType::KW_BREAK] = "break";
  generator[TokenType::KW_CONTINUE] = "continue";
  generator[TokenType::KW_ELSE] = "else";
  generator[TokenType::KW_IF] = "if";
  generator[TokenType::KW_RETURN] = "return";
  generator[TokenType::KW_WHILE] = "while";

  generator[TokenType::KW_IMPORT] = "import";
  generator[TokenType::KW_EXPORT] = "export";
  generator[TokenType::KW_EXTERN] = "extern";
  generator[TokenType::KW_NAMESPACE] = "namespace";
  generator[TokenType::KW_TYPE] = "type";

  // literals
  generator[TokenType::KW_NULLPTR] = "nullptr";
  generator[TokenType::KW_FALSE] = "false";
  generator[TokenType::KW_TRUE] = "true";

  // types
  generator[TokenType::KW_I8] = "i8";
  generator[TokenType::KW_I16] = "i16";
  generator[TokenType::KW_I32] = "i32";
  generator[TokenType::KW_I64] = "i64";

  generator[TokenType::KW_U8] = "u8";
  generator[TokenType::KW_U16] = "u16";
  generator[TokenType::KW_U32] = "u32";
  generator[TokenType::KW_U64] = "u64";

  generator[TokenType::KW_C8] = "c8";

  generator[TokenType::KW_B8] = "b8";
  // keywords end

  generator[TokenType::NUMBER] = "{digit}+";

  // TODO: string can contain any character + escape sequence
  generator[TokenType::STRING] = "\"({s_char})*\"";

  // operators
  generator[TokenType::EQUAL] = "=";
  generator[TokenType::PLUS] = "\\+";
  generator[TokenType::MINUS] = "\\-";
  generator[TokenType::STAR] = "\\*";
  generator[TokenType::PERCENT] = "%";
  generator[TokenType::NOT] = "!";
  generator[TokenType::AMPERSAND] = "&";
  generator[TokenType::LESS] = "<";
  generator[TokenType::GREATER] = ">";
  generator[TokenType::LESS_EQ] = "<=";
  generator[TokenType::GREATER_EQ] = ">=";
  generator[TokenType::EQUALEQUAL] = "==";
  generator[TokenType::NOTEQUAL] = "!=";
  generator[TokenType::PLUSPLUS] = "\\+\\+";
  generator[TokenType::MINUSMINUS] = "\\-\\-";

  generator[TokenType::ARROW] = "\\->";

  generator[TokenType::COLONCOLON] = "::";

  // special symbols
  generator[TokenType::OPEN_PAREN] = "\\(";
  generator[TokenType::CLOSE_PAREN] = "\\)";
  generator[TokenType::OPEN_BRACE] = "{";
  generator[TokenType::CLOSE_BRACE] = "}";
  generator[TokenType::SEMICOLON] = ";";
  generator[TokenType::COLON] = ":";
  generator[TokenType::COMMA] = ",";
  generator[TokenType::DOT] = ".";

  // special values
  generator[TokenType::WHITESPACE] = "{space}+";
  generator[TokenType::COMMENT] = "//{comment_symbol}*";

  auto absolute_lexis_filepath = Constants::GetBuildFilePath("lexis/lexis.lx");
  generator.build_and_save(absolute_lexis_filepath);

  fmt::print("Stored lexis table in: {:?}.\n", absolute_lexis_filepath.c_str());
}
