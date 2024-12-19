#include "Grammar.h"

#include <unordered_set>

namespace Syntax {
void Grammar::check_non_producing() {
  using PartT = GrammarProductionResult::PartT;

  std::unordered_set<NonTerminal> producing;
  std::unordered_set<NonTerminal> to_process;

  for (const auto& [non_terminal, productions] : productions_) {
    for (const GrammarProductionResult& production :
         productions | std::views::keys) {
      if (production.is_terminal()) {
        to_process.emplace(non_terminal);
        break;
      }
    }
  }

  while (!to_process.empty()) {
    producing.merge(to_process);
    to_process.clear();

    for (const auto& [non_terminal, productions] : productions_) {
      for (const auto& production : productions | std::views::keys) {
        bool is_producing = std::ranges::all_of(
            production.get_parts(), [&producing](const PartT& part) {
              if (std::holds_alternative<Terminal>(part)) {
                return true;
              }

              return producing.contains(std::get<NonTerminal>(part));
            });

        if (is_producing && !producing.contains(non_terminal)) {
          to_process.emplace(non_terminal);
          break;
        }
      }
    }
  }

  if (producing.size() != productions_.size()) {
    throw std::runtime_error(
        "Grammar contains non-producing non-terms. Consider removing "
        "them.");
  }
}

void Grammar::check_unreachable() {
  using PartT = GrammarProductionResult::PartT;

  std::unordered_set<NonTerminal> reachable;
  std::unordered_set<NonTerminal> to_process;

  to_process.emplace(start_);

  while (!to_process.empty()) {
    auto prev_updated = to_process;
    reachable.merge(to_process);
    to_process.clear();

    for (NonTerminal non_terminal : prev_updated) {
      for (const auto& production :
           productions_[non_terminal] | std::views::keys) {
        for (const PartT& part : production.get_parts()) {
          if (std::holds_alternative<Terminal>(part)) {
            continue;
          }

          NonTerminal next = std::get<NonTerminal>(part);
          if (!reachable.contains(next)) {
            to_process.emplace(next);
          }
        }
      }
    }
  }

  if (reachable.size() != productions_.size()) {
    throw std::runtime_error(
        "Grammar contains unreachable non-terms. Consider removing them.");
  }
}

void Grammar::check_epsilon_producing() {
  using PartT = GrammarProductionResult::PartT;

  // find epsilon producing non-terminals
  std::unordered_set<NonTerminal> epsilon_producing;
  std::unordered_set<NonTerminal> to_process;

  for (const auto& [non_terminal, productions] : productions_) {
    for (const auto& production : productions | std::views::keys) {
      if (production.empty()) {
        to_process.emplace(non_terminal);
        break;
      }
    }
  }

  while (!to_process.empty()) {
    epsilon_producing.merge(to_process);
    to_process.clear();

    for (const auto& [non_terminal, productions] : productions_) {
      for (const auto& production : productions | std::views::keys) {
        bool is_epsilon_producing = std::ranges::all_of(
            production.get_parts(), [&epsilon_producing](const PartT& part) {
              if (std::holds_alternative<Terminal>(part)) {
                return false;
              }

              return epsilon_producing.contains(std::get<NonTerminal>(part));
            });

        if (is_epsilon_producing && !epsilon_producing.contains(non_terminal)) {
          to_process.emplace(non_terminal);
          break;
        }
      }
    }
  }

  if (!epsilon_producing.empty()) {
    throw std::runtime_error(
        "Grammar must not contain epsilon producing rules");
  }
}

std::list<GrammarProductionResult> Grammar::generate_reduced_productions(
    const GrammarProductionResult& production,
    const std::unordered_set<NonTerminal>& epsilon_producing) {
  using PartT = GrammarProductionResult::PartT;
  std::list result{GrammarProductionResult{}};

  for (const PartT& part : production.get_parts()) {
    if (std::holds_alternative<Terminal>(part) ||
        !epsilon_producing.contains(std::get<NonTerminal>(part))) {
      for (GrammarProductionResult& result_production : result) {
        result_production.add_part(part);
      }
    } else {
      auto copy = result;
      for (auto& result_production : result) {
        result_production.add_part(part);
      }
      result.splice(result.end(), copy);
    }
  }

  return result;
}

void Grammar::check() {
  check_non_producing();
  check_unreachable();
  check_epsilon_producing();
}

void Grammar::add_rule(NonTerminal from, GrammarProductionResult to,
                       BuilderFunction builder) {
  for (const auto& [production, builder_id] : get_productions_for(from)) {
    if (production == to) {
      return;
    }
  }

  productions_[from].emplace_back(std::move(to), builder);
}
}  // namespace Syntax
