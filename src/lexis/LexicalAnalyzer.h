#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include "lexis/Token.h"
#include "lexis/table/LexicalAutomatonState.h"

namespace Lexis {
class LexicalAnalyzer {
  const std::vector<JumpTableT> jumps_;
  static std::unordered_map<std::string_view, TokenType> keywords;

  std::istream* stream_{nullptr};

  Token get_token_internal() const;

 public:
  explicit LexicalAnalyzer(const std::filesystem::path& path);

  Token get_token() const;
  void set_stream(std::istream& stream);
};
}  // namespace Lexis
