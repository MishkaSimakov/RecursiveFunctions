#pragma once

#include <bitset>
#include <cstdint>
#include <string>
#include <vector>

// for now \0 is treated as EOF
// TODO: maybe separate \0 from EOF
struct Charset {
  using CharacterT = uint8_t;
  static constexpr size_t kCharactersCount = 128;
  static constexpr size_t kEOF = 0; // EOF symbol id

  // this character is guaranteed to not be in charset
  static constexpr CharacterT kReservedCharacter =
      static_cast<CharacterT>(kCharactersCount);
};

inline std::string print_symbol(char symbol) {
  if (std::iscntrl(symbol) != 0) {
    return "\\{" + std::to_string(static_cast<size_t>(symbol)) + "}";
  }

  if (symbol == '\"') {
    return "\\" + std::string(1, symbol);
  }

  return std::string(1, symbol);
}

inline std::vector<std::pair<char, char>> group_symbols(
    std::bitset<Charset::kCharactersCount> symbols) {
  std::vector<std::pair<char, char>> groups;

  for (size_t i = 0; i < Charset::kCharactersCount; ++i) {
    if (symbols[i] && (i == 0 || !symbols[i - 1])) {
      groups.emplace_back(i, 0);
    }
    if (symbols[i] && (i + 1 == Charset::kCharactersCount || !symbols[i + 1])) {
      groups.back().second = i;
    }
  }

  return groups;
}
