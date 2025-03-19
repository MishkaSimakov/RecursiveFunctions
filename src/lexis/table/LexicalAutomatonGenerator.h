#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "lexis/Token.h"

namespace Lexis {
class LexicalAutomatonGenerator {
  constexpr static auto kTokenHasherFn = [](TokenType token) {
    return std::hash<size_t>()(static_cast<size_t>(token));
  };

  std::unordered_map<TokenType, std::string, decltype(kTokenHasherFn)> tokens_;
  std::unordered_map<std::string, std::string> helpers_;

 public:
  std::string& operator[](TokenType token) { return tokens_[token]; }

  std::string& operator[](std::string_view string) {
    return helpers_[std::string(string)];
  }

  void build_and_save(const std::filesystem::path& save_path);
};
}  // namespace Lexis
