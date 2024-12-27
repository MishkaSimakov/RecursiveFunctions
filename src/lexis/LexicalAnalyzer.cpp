#include "LexicalAnalyzer.h"

#include <fstream>
#include <string_view>
#include <variant>

#include "lexis/Charset.h"
#include "lexis/table/LexicalTableSerializer.h"

namespace Lexis {
std::unordered_map<std::string_view, TokenType> LexicalAnalyzer::keywords{
    {"import", TokenType::KW_IMPORT}};

Token LexicalAnalyzer::get_token_internal() const {
  size_t current_state = 0;
  std::string token_value;

  while (true) {
    int symbol = stream_->get();

    if (stream_->eof()) {
      return Token{TokenType::END};
    }
    if (symbol >= Charset::kCharactersCount) {
      return Token{TokenType::ERROR};
    }

    token_value += static_cast<char>(symbol);
    auto jump = jumps_[current_state][symbol];

    if (std::holds_alternative<NextStateJump>(jump)) {
      current_state = std::get<NextStateJump>(jump).state_id;
      continue;
    }

    if (std::holds_alternative<FinishJump>(jump)) {
      FinishJump finish_jump = std::get<FinishJump>(jump);
      stream_->seekg(-static_cast<off_t>(finish_jump.forward_shift),
                   std::ios_base::cur);
      token_value.resize(token_value.size() - finish_jump.forward_shift);

      return Token{TokenType{finish_jump.token}, token_value};
    }

    return Token{TokenType::ERROR};
  }
}

LexicalAnalyzer::LexicalAnalyzer(const std::filesystem::path& path)
    : jumps_([&path] {
        std::ifstream is(path);
        return LexicalTableSerializer::deserialize(is);
      }()) {}

Token LexicalAnalyzer::get_token() const {
  Token token;
  do {
    token = get_token_internal();
  } while (token.type == TokenType::WHITESPACE ||
           token.type == TokenType::COMMENT);

  return token;
}

void LexicalAnalyzer::set_stream(std::istream& stream) { stream_ = &stream; }
}  // namespace Lexis
