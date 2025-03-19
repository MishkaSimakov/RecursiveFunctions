#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "lexis/Token.h"

namespace Lexis {
struct TokenTypeHasher {
  size_t operator()(TokenType token) const {
    return std::hash<size_t>()(static_cast<size_t>(token));
  }
};

class LexicalAutomatonGenerator {
  std::unordered_map<TokenType, std::string, TokenTypeHasher> tokens_;
  std::unordered_map<std::string, std::string> helpers_;
  std::unordered_set<TokenType, TokenTypeHasher> weak_tokens_;

 public:
  std::string& operator[](TokenType token) { return tokens_[token]; }

  std::string& operator[](std::string_view string) {
    return helpers_[std::string(string)];
  }

  void mark_token_as_weak(TokenType token) { weak_tokens_.insert(token); }

  void build_and_save(const std::filesystem::path& save_path);
};
}  // namespace Lexis
