#include "LexicalAnalyzer.h"

#include <fstream>
#include <variant>

#include "lexis/Charset.h"
#include "lexis/table/LexicalTableSerializer.h"

namespace Lexis {
Token LexicalAnalyzer::get_token_internal(SourceLocation location) const {
  size_t current_state = 0;
  SourceLocation begin = location;
  SourceLocation cur_loc = location;

  const SourceLocation end_loc = source_view_.end_location();

  if (cur_loc == end_loc) {
    return Token{TokenType::END, SourceRange{cur_loc, cur_loc}};
  }

  while (true) {
    bool reached_end = cur_loc == end_loc;
    char symbol = reached_end ? '\0' : source_view_[cur_loc];
    ++cur_loc.pos_id;

    if (symbol >= Charset::kCharactersCount) {
      return Token{TokenType::ERROR, SourceRange{cur_loc, cur_loc}};
    }

    auto jump = jumps_[current_state][symbol];

    if (std::holds_alternative<NextStateJump>(jump)) {
      current_state = std::get<NextStateJump>(jump).state_id;
      continue;
    }

    if (std::holds_alternative<FinishJump>(jump)) {
      FinishJump finish_jump = std::get<FinishJump>(jump);
      cur_loc.pos_id -= finish_jump.forward_shift;

      return {Token{TokenType{finish_jump.token}, {begin, cur_loc}}};
    }

    return Token{TokenType::ERROR, {begin, cur_loc}};
  }
}

LexicalAnalyzer::LexicalAnalyzer(const std::filesystem::path& path)
    : jumps_([&path] {
        std::ifstream is(path);

        if (!is) {
          throw std::runtime_error("Failed to open lexis table.");
        }

        return LexicalTableSerializer::deserialize(is);
      }()) {}

void LexicalAnalyzer::set_source_view(SourceView view) {
  source_view_ = view;
}

Token LexicalAnalyzer::get_token() {
  Token token = peek_token();
  location_ = token.source_range.end;
  future_token_.reset();
  return token;
}

Token LexicalAnalyzer::peek_token() const {
  if (!future_token_.has_value()) {
    SourceLocation location = location_;
    do {
      future_token_ = get_token_internal(location);
      location = future_token_->source_range.end;
    } while (future_token_->type == TokenType::WHITESPACE ||
             future_token_->type == TokenType::COMMENT);
  }

  return future_token_.value();
}
}  // namespace Lexis
