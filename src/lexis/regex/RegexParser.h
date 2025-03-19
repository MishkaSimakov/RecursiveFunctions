#pragma once
#include <memory>
#include <string_view>

#include "RegexNodes.h"

class RegexParser {
  static bool is_operator(char symbol);

  static std::string_view::iterator get_matching_paren(std::string_view regex);
  static std::string_view::iterator get_closing_bracket(std::string_view regex);

  static std::unique_ptr<RegexNode> parse_character_class(
      std::string_view regex);
  static std::unique_ptr<RegexNode> parse_recursively(std::string_view regex);

  /*
   * Operators such as: +*|[]()-^ are not allowed to appear as regular symbols.
   * They must be escaped using "\". This method reads one symbol and correctly
   * handles escaped symbols.
   */
  static std::pair<char, size_t> read_symbol(std::string_view regex);

 public:
  RegexParser() = default;

  static std::unique_ptr<RegexNode> parse(std::string_view regex);
};
