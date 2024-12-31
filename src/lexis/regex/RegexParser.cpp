#include "RegexParser.h"

#include <lexis/Charset.h>

#include <locale>
#include <set>
#include <vector>

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

std::unique_ptr<RegexNode> RegexParser::parse_symbols_range(
    std::string_view regex) {
  std::vector<std::pair<size_t, size_t>> additions;
  std::set<size_t> exclusions;

  for (size_t i = 0; i < regex.size();) {
    if (regex[i] == '^') {
      if (i + 1 >= regex.size()) {
        throw std::runtime_error(
            "In symbols range: symbol must be presented after exclusion sign "
            "(^).");
      }

      exclusions.insert(regex[i + 1]);
      i += 2;
      continue;
    }

    if (i + 1 < regex.size() && regex[i + 1] == '-') {
      if (i + 2 >= regex.size()) {
        throw std::runtime_error("In symbols range: unclosed range.");
      }

      additions.emplace_back(regex[i], regex[i + 2]);

      i += 3;
    } else {
      additions.emplace_back(regex[i], regex[i]);
      ++i;
    }
  }

  if (additions.empty() && !exclusions.empty()) {
    additions.emplace_back(0, Charset::kCharactersCount - 1);
  }

  std::bitset<Charset::kCharactersCount> result;
  for (auto [from, to] : additions) {
    for (size_t i = from; i <= to; ++i) {
      result[i] = true;
    }
  }
  for (size_t symbol : exclusions) {
    result[symbol] = false;
  }

  return std::make_unique<SymbolNode>(result);
}

std::unique_ptr<RegexNode> RegexParser::parse_recursively(
    std::string_view regex) {
  // search for top-level +
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
    left = parse_symbols_range({regex.begin() + 1, symbols_range_end});
    regex.remove_prefix(symbols_range_end - regex.begin() + 1);
  } else if (regex.front() == '\\') {
    // escaped characters are treated like plain symbols
    regex.remove_prefix(1);
    left = std::make_unique<SymbolNode>(regex.front());
    regex.remove_prefix(1);
  } else {
    left = std::make_unique<SymbolNode>(regex.front());
    regex.remove_prefix(1);
  }

  if (regex.empty()) {
    return left;
  }

  auto right = parse_recursively(regex);
  return std::make_unique<ConcatenationNode>(std::move(left), std::move(right));
}

std::unique_ptr<RegexNode> RegexParser::parse(std::string_view regex) {
  return parse_recursively(regex);
}
