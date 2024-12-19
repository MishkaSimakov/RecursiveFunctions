#pragma once

#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "BuildersRegistry.h"
#include "GrammarProduction.h"

namespace Syntax {
class Grammar {
  size_t nonterminals_count_{0};
  // non-terminal id / productions with it
  std::unordered_map<
      NonTerminal,
      std::vector<std::pair<GrammarProductionResult, BuilderFunction>>>
      productions_;
  NonTerminal start_{0};

  void erase_unlisted(const std::unordered_set<NonTerminal>& keep);

  static std::list<GrammarProductionResult> generate_reduced_productions(
      const GrammarProductionResult& production,
      const std::unordered_set<NonTerminal>& epsilon_producing);

  void check_non_producing();
  void check_unreachable();
  void check_epsilon_producing();

 public:
  const auto& get_productions() const { return productions_; }

  NonTerminal register_nonterm() { return NonTerminal{nonterminals_count_++}; }

  std::span<const std::pair<GrammarProductionResult, BuilderFunction>>
  get_productions_for(const NonTerminal& non_terminal) const {
    auto itr = productions_.find(non_terminal);

    if (itr == productions_.end()) {
      return {};
    }

    return itr->second;
  }

  NonTerminal get_start() const { return start_; }
  void set_start(NonTerminal new_start) { start_ = new_start; }
  auto get_start_productions() const { return get_productions_for(start_); }

  void check();

  void add_rule(NonTerminal from, GrammarProductionResult to,
                BuilderFunction builder);
};
}  // namespace Syntax
