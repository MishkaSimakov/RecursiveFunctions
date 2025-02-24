#include <gtest/gtest.h>

#include <ranges>

#include "RegexParserTestCase.h"

TEST_F(RegexParserTestCase, test_it_parse_simple_concatenation) {
  std::vector<std::string> regexes = {
      "abbbaa",         "(a)",   "a",          "a(b)",
      "ab(ababa)abbbb", "bbbbb", "bababababa", "(a)bbababa"};

  for (auto& regex : regexes) {
    auto result = RegexParser::parse(regex);

    std::erase_if(regex,
                  [](char symbol) { return symbol == '(' || symbol == ')'; });

    ASSERT_TRUE(is_equals_to_string(*result, regex));
  }
}

TEST_F(RegexParserTestCase, test_it_parse_simple_or_expression) {
  std::string regex = "abb|bba";

  auto result = RegexParser::parse(regex);

  {
    const auto& or_node = node_cast<OrNode>(*result);
    ASSERT_TRUE(is_equals_to_string(*or_node.left, "abb") &&
                is_equals_to_string(*or_node.right, "bba"));
  }

  regex = "babbabbab|babbab|bab";
  result = RegexParser::parse(regex);
  {
    const auto& first_or = node_cast<OrNode>(result);
    const auto& second_or = node_cast<OrNode>(first_or.right);

    ASSERT_TRUE(is_equals_to_string(*first_or.left, "babbabbab") &&
                is_equals_to_string(*second_or.left, "babbab") &&
                is_equals_to_string(*second_or.right, "bab"));
  }
}

TEST_F(RegexParserTestCase, test_it_parse_star) {
  {
    auto result = RegexParser::parse("(a)*");

    const auto& star_node = node_cast<StarNode>(result);
    is_equals_to_string(*star_node.child, "a");
  }

  {
    auto result = RegexParser::parse("(ab)*(ba)*");

    const auto& concatenation = node_cast<ConcatenationNode>(result);
    const auto& left_child = node_cast<StarNode>(concatenation.left);
    const auto& right_child = node_cast<StarNode>(concatenation.right);

    ASSERT_TRUE(is_equals_to_string(*left_child.child, "ab") &&
                is_equals_to_string(*right_child.child, "ba"));
  }

  {
    auto result = RegexParser::parse("(ab|ba)*|a");

    const auto& or_node = node_cast<OrNode>(result);

    ASSERT_TRUE(is_equals_to_string(*or_node.right, "a"));

    const auto& left_child = node_cast<StarNode>(or_node.left);
    const auto& left_child_or = node_cast<OrNode>(left_child.child);

    ASSERT_TRUE(is_equals_to_string(*left_child_or.left, "ab") &&
                is_equals_to_string(*left_child_or.right, "ba"));
  }
}

TEST_F(RegexParserTestCase, simple_charclass_test) {
  std::unordered_map<std::string_view, std::string_view> tests = {
      {"[abcd]", "abcd"},   {"[a-d]", "abcd"},       {"[a-dxy]", "abcdxy"},
      {"[a\\-xy]", "a-xy"}, {"[a-dx-z]", "abcdxyz"},
  };

  for (auto [regex, expected] : tests) {
    auto result = RegexParser::parse(regex);
    ASSERT_TRUE(is_group_equal(node_cast<SymbolNode>(result), expected))
        << fmt::format("Test for regex {:?} failed.", regex);
  }
}

TEST_F(RegexParserTestCase, negated_charclass_test) {
  {
    auto result = RegexParser::parse("[^xyz]");
    GroupMatchT expected;
    expected.set();

    expected['x'] = false;
    expected['y'] = false;
    expected['z'] = false;

    ASSERT_EQ(node_cast<SymbolNode>(result).match, expected);
  }

  {
    auto result = RegexParser::parse("[^a-dx]");
    GroupMatchT expected;
    expected.set();

    expected['a'] = false;
    expected['b'] = false;
    expected['c'] = false;
    expected['d'] = false;
    expected['x'] = false;

    ASSERT_EQ(node_cast<SymbolNode>(result).match, expected);
  }
}

TEST_F(RegexParserTestCase, escaped_characters_test) {
  {
    auto result = RegexParser::parse("[\\^ab\\*]");
    ASSERT_TRUE(is_group_equal(node_cast<SymbolNode>(result), "^ab*"));
  }

  {
    auto result = RegexParser::parse("(\\+)+\\||\\(");

    auto& or_node = node_cast<OrNode>(result);

    // left child
    auto& concatenation_node = node_cast<ConcatenationNode>(or_node.left);
    ASSERT_TRUE(is_group_equal(
        node_cast<SymbolNode>(concatenation_node.right).match, "|"));

    auto& plus_node = node_cast<PlusNode>(concatenation_node.left);
    ASSERT_TRUE(
        is_group_equal(node_cast<SymbolNode>(plus_node.child).match, "+"));

    // right child
    auto& right_child = node_cast<SymbolNode>(or_node.right);
    ASSERT_TRUE(is_group_equal(right_child.match, "("));
  }

  {
    auto result = RegexParser::parse(R"(\\)");
    auto& symbol_node = node_cast<SymbolNode>(result);
    ASSERT_TRUE(is_group_equal(symbol_node.match, "\\"));
  }

  {
    auto result = RegexParser::parse("(\\))+");
    auto& plus_node = node_cast<PlusNode>(result);
    auto& symbol_node = node_cast<SymbolNode>(plus_node.child);
    ASSERT_TRUE(is_group_equal(symbol_node.match, ")"));
  }

  {
    auto result = RegexParser::parse("[\\]]");
    auto& symbol_node = node_cast<SymbolNode>(result);
    ASSERT_TRUE(is_group_equal(symbol_node.match, "]"));
  }
}
