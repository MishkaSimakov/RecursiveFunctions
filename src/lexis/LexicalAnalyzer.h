#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include "lexis/Token.h"
#include "lexis/table/LexicalAutomatonState.h"
#include "sources/SourceManager.h"

namespace Lexis {
class LexicalAnalyzer {
  const std::vector<JumpTableT> jumps_;
  static std::unordered_map<std::string_view, TokenType> keywords;

  SourceLocation location_{};
  SourceView source_view_;
  std::optional<Token> current_token_;

  Token get_token_internal(SourceLocation location) const;

 public:
  LexicalAnalyzer(const std::filesystem::path& table_path);

  void set_source_view(SourceView view);

  Token next_token();
  Token current_token() const;
};
}  // namespace Lexis
