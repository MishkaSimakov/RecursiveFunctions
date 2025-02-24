#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "lexis/regex/RegexParser.h"

class RegexParserTestCase : public ::testing::Test {
 private:
  size_t is_equals_to_string_recursive(RegexNode& node,
                                       std::string_view string) {
    if (node.is_of_type<SymbolNode>()) {
      auto match = node_cast<SymbolNode>(node).match;
      bool match_symbol = !match.none();

      match.flip(string.front());
      bool match_nothing_else = match.none();

      return match_symbol && match_nothing_else;
    }

    if (node.is_of_type<ConcatenationNode>()) {
      const auto& concatenation = node_cast<ConcatenationNode>(node);

      size_t left = is_equals_to_string_recursive(*concatenation.left, string);
      string.remove_prefix(left);
      size_t right =
          is_equals_to_string_recursive(*concatenation.right, string);

      return left + right;
    }

    return 0;
  }

 protected:
  using GroupMatchT = decltype(SymbolNode::match);

  bool is_group_equal(const SymbolNode& node, std::string_view symbols) {
    GroupMatchT expected;
    for (char symbol : symbols) {
      expected[symbol] = true;
    }

    return expected == node.match;
  }

  template <typename U, typename T>
    requires std::is_base_of_v<RegexNode, T> && std::is_base_of_v<T, U>
  static const U& node_cast(const T& node) {
    try {
      return dynamic_cast<const U&>(node);
    } catch (std::bad_cast&) {
      throw std::runtime_error(fmt::format("Error when casting {:?} to {:?}",
                                           typeid(T).name(), typeid(U).name()));
    }
  }

  template <typename U, typename T>
    requires std::is_base_of_v<RegexNode, T> && std::is_base_of_v<T, U>
  static const U& node_cast(const std::unique_ptr<T>& node) {
    try {
      return dynamic_cast<const U&>(*node);
    } catch (std::bad_cast&) {
      throw std::runtime_error(fmt::format("Error when casting {:?} to {:?}",
                                           typeid(T).name(), typeid(U).name()));
    }
  }

  bool is_equals_to_string(RegexNode& node, std::string_view string) {
    return is_equals_to_string_recursive(node, string) == string.size();
  }
};
