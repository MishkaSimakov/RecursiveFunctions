#include <iostream>

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

  // keywords
  // keywords
  generator[TokenType::KW_BREAK] = "break";
  generator[TokenType::KW_CONTINUE] = "continue";
  generator[TokenType::KW_ELSE] = "else";
  generator[TokenType::KW_IMPORT] = "import";
  generator[TokenType::KW_EXPORT] = "export";
  generator[TokenType::KW_EXTERN] = "extern";
  generator[TokenType::KW_IF] = "if";
  generator[TokenType::KW_NAMESPACE] = "namespace";
  generator[TokenType::KW_RETURN] = "return";
  generator[TokenType::KW_USING] = "using";
  generator[TokenType::KW_WHILE] = "while";

  // literals
  generator[TokenType::KW_NULLPTR] = "nullptr";
  generator[TokenType::KW_FALSE] = "false";
  generator[TokenType::KW_TRUE] = "true";

  // types
  generator[TokenType::KW_U64] = "u64";
  generator[TokenType::KW_I64] = "i64";
  generator[TokenType::KW_F64] = "f64";
  generator[TokenType::KW_BOOL] = "bool";
  generator[TokenType::KW_CHAR] = "char";
  generator[TokenType::KW_VOID] = "void";
  // keywords end

  generator[TokenType::IDENTIFIER] = "[a-zA-Z_]([a-zA-Z0-9_])*";
  generator[TokenType::NUMBER] = "{digit}+";

  // TODO: string can contain any character + escape sequence
  generator[TokenType::STRING] = "\"({s_char})*\"";

  // operators
  generator[TokenType::EQUAL] = "=";
  generator[TokenType::PLUS] = "\\+";
  generator[TokenType::MINUS] = "\\-";
  generator[TokenType::STAR] = "\\*";
  generator[TokenType::PERCENT] = "%";
  generator[TokenType::AMPERSAND] = "&";
  generator[TokenType::LESS] = "<";
  generator[TokenType::GREATER] = ">";
  generator[TokenType::LESS_EQ] = "<=";
  generator[TokenType::GREATER_EQ] = ">=";
  generator[TokenType::EQUALEQUAL] = "==";
  generator[TokenType::NOTEQUAL] = "!=";

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

  // special values
  generator[TokenType::WHITESPACE] = "{space}+";
  generator[TokenType::COMMENT] = "//{comment_symbol}*";

  auto absolute_lexis_filepath =
      std::filesystem::path(BASE_PATH) / Constants::lexis_filepath;
  generator.build_and_save(absolute_lexis_filepath);

  std::cout << "Stored lexis table in: " << absolute_lexis_filepath
            << std::endl;
}
