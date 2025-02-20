#include "GrammarGenerator.h"

#include <fmt/format.h>
#include <syntax/lr/LRTableBuilder.h>

#include <fstream>
#include <vector>

#include "Grammar.h"

namespace Syntax {
namespace {
using TokenType = Lexis::TokenType;

struct Production {
  std::vector<std::string> result;
  std::string builder;
};

struct Rule {
  std::string name;
  std::vector<Production> productions;
};

std::string_view skip_spaces(std::string_view view) {
  while (!view.empty() && std::isspace(view.front()) != 0) {
    view.remove_prefix(1);
  }
  return view;
}

std::vector<Rule> read_grammar(const std::filesystem::path& path) {
  std::vector<Rule> result;
  std::ifstream is(path);

  if (!is) {
    throw std::runtime_error("Failed to open grammar file");
  }

  size_t line_index = 0;
  std::string buffer;
  bool start_new_rule = true;
  while (std::getline(is, buffer)) {
    ++line_index;

    std::string_view line = buffer;
    line = skip_spaces(line);
    while (!line.empty() && std::isspace(line.back()) != 0) {
      line.remove_suffix(1);
    }

    if (line.empty()) {
      start_new_rule = true;
      continue;
    }
    if (line.starts_with("//")) {
      continue;
    }

    if (start_new_rule) {
      if (line.back() != ':') {
        throw std::runtime_error(fmt::format(
            "Error in grammar file: line {} - expected colon.", line_index));
      }

      line.remove_suffix(1);
      result.emplace_back();
      result.back().name = line;
      start_new_rule = false;
      continue;
    }

    // line must look like "a b c [call]"
    auto& production = result.back().productions.emplace_back();
    while (line.front() != '[') {
      auto part_begin = line.begin();
      while (std::isspace(line.front()) == 0) {
        line.remove_prefix(1);
      }
      auto part_end = line.begin();

      production.result.emplace_back(std::string(part_begin, part_end));

      line = skip_spaces(line);
    }

    if (line.back() != ']') {
      throw std::runtime_error(fmt::format(
          "Error in grammar file: line {} - expected closing bracket.",
          line_index));
    }

    line.remove_suffix(1);
    line.remove_prefix(1);
    production.builder = line;
  }

  return result;
}

Grammar parse_grammar(const std::vector<Rule>& text_grammar) {
  Grammar result;

  std::unordered_map<std::string, NonTerminal> nonterminals;
  for (const auto& [name, _] : text_grammar) {
    nonterminals.emplace(name, result.register_nonterm());
  }

  result.set_start(nonterminals.at(text_grammar.front().name));

  for (const auto& [name, productions] : text_grammar) {
    for (const auto& [parts, _] : productions) {
      GrammarProductionResult grammar_production;

      // special syntax for empty production
      // if (parts.size() == 1 && parts.front() == "empty") {
        // break;
      // }

      for (const auto& part : parts) {
        auto token = TokenType::from_string(part);

        if (token.has_value()) {
          grammar_production += Terminal{token.value()};
        } else if (nonterminals.contains(part)) {
          grammar_production += nonterminals.at(part);
        } else {
          throw std::runtime_error(fmt::format(
              "Each part of production result must be either token or valid "
              "nonterm. Met {:?} in production {:?}.",
              part, name));
        }
      }

      result.add_rule(nonterminals.at(name), std::move(grammar_production));
    }
  }

  return result;
}

void generate_function_file(const std::filesystem::path& path,
                            const std::vector<Rule>& text_grammar) {
  std::ofstream os(path);

  if (!os) {
    throw std::runtime_error("Failed to open functions file.");
  }

  os << "const std::function<std::unique_ptr<ASTNode>(ASTBuildContext*, "
        "SourceRange, std::span<std::unique_ptr<ASTNode>>)> builders[] {\n";

  for (const auto& [name, productions] : text_grammar) {
    for (const auto& [_, builder] : productions) {
      os << "&ASTBuildContext::" << builder << ",\n";
    }
  }

  os << "};\n";
}
}  // namespace

void GrammarGenerator::generate_grammar(
    const std::filesystem::path& input_path,
    const std::filesystem::path& table_path,
    const std::filesystem::path& builders_path) {
  auto text_grammar = read_grammar(input_path);
  auto grammar = parse_grammar(text_grammar);

  try {
    auto builder = LRTableBuilder(std::move(grammar));
    builder.save_to(table_path);
  } catch (ActionsConflictException exception) {
    std::cout << exception.what() << std::endl;
  }

  // now we generate file with builders list
  generate_function_file(builders_path, text_grammar);
}
}  // namespace Syntax
