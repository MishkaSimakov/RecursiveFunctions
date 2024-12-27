#pragma once

#include <cstdint>
#include <string>

struct Charset {
  using CharacterT = uint8_t;
  static constexpr size_t kCharactersCount = 128;

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
