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
  generator["comment_symbol"] = "[a-zA-Z0-9 \t\r\v\f]";

  // keywords
  generator[TokenType::KW_IMPORT] = "import";
  generator[TokenType::KW_INT] = "int";
  generator[TokenType::KW_RETURN] = "return";

  generator[TokenType::IDENTIFIER] = "{letter}({letter}|{digit})*";
  generator[TokenType::NUMBER] = "{digit}+";

  // TODO: string can contain any character + escape sequence
  generator[TokenType::STRING] = "\"({letter}|{digit})*\"";

  // operators
  generator[TokenType::EQUAL] = "=";
  generator[TokenType::PLUS] = "\\+";
  generator[TokenType::LESS] = "<";
  generator[TokenType::GREATER] = ">";
  generator[TokenType::LESS_EQ] = "<=";
  generator[TokenType::GREATER_EQ] = ">=";

  generator[TokenType::ARROW] = "->";

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
