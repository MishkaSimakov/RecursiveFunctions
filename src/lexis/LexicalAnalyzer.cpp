#include "LexicalAnalyzer.h"

#include <fstream>
#include <string_view>
#include <variant>

#include "lexis/Charset.h"
#include "lexis/table/LexicalTableSerializer.h"

namespace Lexis {
Token LexicalAnalyzer::get_token_internal() {
  size_t current_state = 0;
  SourceLocation begin = location_;

  auto view = source_manager_.get_file_view(location_);
  if (view.empty()) {
    return Token{TokenType::END};
  }

  while (true) {
    // we add \0 in the end to finish tokens lookahead
    if (view.empty()) {
      view = "\0";
    }

    char symbol = view.front();
    view.remove_prefix(1);
    ++location_.pos_id;

    if (symbol >= Charset::kCharactersCount) {
      return Token{TokenType::ERROR};
    }

    auto jump = jumps_[current_state][symbol];

    if (std::holds_alternative<NextStateJump>(jump)) {
      current_state = std::get<NextStateJump>(jump).state_id;
      continue;
    }

    if (std::holds_alternative<FinishJump>(jump)) {
      FinishJump finish_jump = std::get<FinishJump>(jump);
      seek(-static_cast<off_t>(finish_jump.forward_shift));

      return Token{TokenType{finish_jump.token}, {begin, location_}};
    }

    return Token{TokenType::ERROR, {begin, location_}};
  }
}

LexicalAnalyzer::LexicalAnalyzer(const std::filesystem::path& path,
                                 const SourceManager& source_manager)
    : jumps_([&path] {
        std::ifstream is(path);

        if (!is) {
          throw std::runtime_error("Failed to open lexis table.");
        }

        return LexicalTableSerializer::deserialize(is);
      }()),
      source_manager_(source_manager) {}

void LexicalAnalyzer::set_location(SourceLocation location) {
  location_ = location;
}

void LexicalAnalyzer::seek(off_t offset) { location_.pos_id += offset; }

Token LexicalAnalyzer::get_token() {
  Token token;
  do {
    token = get_token_internal();
  } while (token.type == TokenType::WHITESPACE ||
           token.type == TokenType::COMMENT);

  return token;
}
}  // namespace Lexis
