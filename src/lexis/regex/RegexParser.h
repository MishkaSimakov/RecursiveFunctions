#pragma once
#include <memory>
#include <string_view>

#include "RegexNodes.h"

class RegexParser {
  static std::string_view::iterator get_matching_paren(std::string_view regex);
  static std::string_view::iterator get_closing_bracket(std::string_view regex);

  static std::unique_ptr<RegexNode> parse_symbols_range(std::string_view regex);
  static std::unique_ptr<RegexNode> parse_recursively(std::string_view regex);

 public:
  RegexParser() = default;

  static std::unique_ptr<RegexNode> parse(std::string_view regex);
};
