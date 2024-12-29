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
  const SourceManager& source_manager_;

  Token get_token_internal();

 public:
  LexicalAnalyzer(const std::filesystem::path& table_path,
                  const SourceManager& source_manager);

  void set_location(SourceLocation location);
  void seek(off_t offset);

  Token get_token();
};
}  // namespace Lexis
