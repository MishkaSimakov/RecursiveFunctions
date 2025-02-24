#include "RegexParser.h"

#include <lexis/Charset.h>

#include <locale>
#include <set>
#include <vector>

bool RegexParser::is_operator(char symbol) {
  switch (symbol) {
    case '+':
    case '*':
    case '|':
    case '[':
    case ']':
    case '(':
    case ')':
    case '-':
    case '^':
      return true;
    default:
      return false;
  }
}

std::string_view::iterator RegexParser::get_matching_paren(
    std::string_view regex) {
  int balance = 0;
  for (auto itr = regex.begin(); itr != regex.end(); ++itr) {
    if (*itr == '(') {
      ++balance;
    } else if (*itr == ')') {
      --balance;
    } else if (*itr == '[') {
      itr = get_closing_bracket({itr, regex.end()});
    }

    if (balance == 0) {
      return itr;
    }
  }

  return regex.end();
}

std::string_view::iterator RegexParser::get_closing_bracket(
    std::string_view regex) {
  return std::find(regex.begin(), regex.end(), ']');
}

std::unique_ptr<RegexNode> RegexParser::parse_character_class(
    std::string_view regex) {
  std::vector<std::pair<size_t, size_t>> ranges;

  if (regex.empty()) {
    throw std::runtime_error("Character class must be non-empty.");
  }

  bool negated = false;
  if (regex.front() == '^') {
    negated = true;
    regex.remove_prefix(1);
  }

  while (!regex.empty()) {
    auto [begin, offset] = read_symbol(regex);
    regex.remove_prefix(offset);

    char end = begin;

    if (!regex.empty() && regex.front() == '-') {
      if (regex.size() < 2) {
        throw std::runtime_error(
            "Plain symbol was expected after hyphen but nothing was provided.");
      }

      // remove hyphen
      regex.remove_prefix(1);

      std::tie(end, offset) = read_symbol(regex);
      regex.remove_prefix(offset);
    }

    ranges.emplace_back(begin, end);
  }

  std::bitset<Charset::kCharactersCount> result;
  if (negated) {
    result.set();
  }

  for (auto [from, to] : ranges) {
    for (size_t i = from; i <= to; ++i) {
      result[i].flip();
    }
  }

  return std::make_unique<SymbolNode>(result);
}

std::unique_ptr<RegexNode> RegexParser::parse_recursively(
    std::string_view regex) {
  // search for top-level |
  for (auto itr = regex.begin(); itr != regex.end(); ++itr) {
    // skip escaped characters
    if (*itr == '\\') {
      ++itr;
      continue;
    }

    if (*itr == '(') {
      itr = get_matching_paren({itr, regex.end()});

      if (itr == regex.end()) {
        throw std::runtime_error("Unmatched parenthesis.");
      }
    } else if (*itr == '|') {
      auto right = parse_recursively({std::next(itr), regex.end()});
      auto left = parse_recursively({regex.begin(), itr});

      return std::make_unique<OrNode>(std::move(left), std::move(right));
    }
  }

  // if there isn't any "|" inside we should just concatenate each part
  if (regex.empty()) {
    throw std::runtime_error("Empty part in regex.");
  }

  // we split regex into two parts: left right and parse them recursively
  std::unique_ptr<RegexNode> left;

  if (regex.front() == '(') {
    auto end_itr = get_matching_paren(regex);

    if (end_itr == regex.end()) {
      throw std::runtime_error("Unmatched parenthesis.");
    }
    if (end_itr == regex.end() || end_itr == std::next(regex.begin())) {
      throw std::runtime_error("Empty parentheses are not allowed.");
    }

    left = parse_recursively({std::next(regex.begin()), end_itr});

    ++end_itr;
    while (*end_itr == '*' || *end_itr == '+') {
      if (*end_itr == '*') {
        left = std::make_unique<StarNode>(std::move(left));
      } else {
        left = std::make_unique<PlusNode>(std::move(left));
      }

      ++end_itr;
    }

    regex.remove_prefix(end_itr - regex.begin());
  } else if (regex.front() == '[') {
    auto symbols_range_end = get_closing_bracket(regex);
    left = parse_character_class({regex.begin() + 1, symbols_range_end});
    regex.remove_prefix(symbols_range_end - regex.begin() + 1);
  } else {
    auto [symbol, offset] = read_symbol(regex);
    left = std::make_unique<SymbolNode>(symbol);
    regex.remove_prefix(offset);
  }

  if (regex.empty()) {
    return left;
  }

  auto right = parse_recursively(regex);
  return std::make_unique<ConcatenationNode>(std::move(left), std::move(right));
}

std::pair<char, size_t> RegexParser::read_symbol(std::string_view regex) {
  if (regex.empty()) {
    throw std::runtime_error(
        "Plain symbol was expected but nothing was provided.");
  }

  if (is_operator(regex.front())) {
    throw std::runtime_error(
        "Operators are not allowed where plain symbol is expected.");
  }

  if (regex.front() != '\\') {
    return {regex.front(), 1};
  }

  if (regex.size() < 2) {
    throw std::runtime_error(
        "Symbol was expected after escape character but nothing was provided.");
  }

  return {regex[1], 2};
}

std::unique_ptr<RegexNode> RegexParser::parse(std::string_view regex) {
  return parse_recursively(regex);
}
