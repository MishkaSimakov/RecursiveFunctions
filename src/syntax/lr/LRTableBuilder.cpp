#include "LRTableBuilder.h"

#include <fstream>
#include <iostream>

#include "LRTableSerializer.h"
#include "lexis/Token.h"
#include "syntax/grammar/GrammarProduction.h"
#include "utils/Hashers.h"

namespace Syntax {
template <typename K, typename V>
  requires requires(V value) {
    { value.empty() } -> std::same_as<bool>;
  }
void EraseEmpty(std::unordered_map<K, V>& map) {
  std::erase_if(map, [](const std::pair<const K, V>& element) {
    return element.second.empty();
  });
}

std::unordered_map<ssize_t, State> LRTableBuilder::group_by_next(
    const State& state) {
  std::unordered_map<ssize_t, State> result;

  for (const auto& [position, follow] : state) {
    auto copy = position;

    ssize_t next_id;

    if (copy.iterator == copy.end_iterator()) {
      next_id = GrammarProductionResult::cRuleEndNumber;
    } else {
      next_id = copy.iterator.as_number();
      ++copy.iterator;
    }

    result[next_id].emplace(copy, follow);
  }

  return result;
}

void LRTableBuilder::build_first_table() {
  auto update_data = [this] {
    std::unordered_map<NonTerminal, TokensBitset> update;

    for (const auto& [non_terminal, productions] : grammar_.get_productions()) {
      auto& nonterm_update = update[non_terminal];

      for (const auto& production : productions | std::views::keys) {
        if (production.empty()) {
          nonterm_update.add(Lexis::TokenType::END);
          continue;
        }

        auto itr = production.cbegin();

        // here we must consider empty productions
        // for example A -> B C a, where B -> epsilon, C -> epsilon
        // in this case a \in First(A)
        for (; itr != production.cend(); ++itr) {
          if (itr.is_terminal()) {
            nonterm_update.add(itr.access_terminal());
            break;
          } else {
            auto first = first_[itr.access_nonterminal()];
            nonterm_update.add(first);

            if (!first.contains(Lexis::TokenType::END)) {
              break;
            }
          }
        }
      }

      // include only updated ones
      nonterm_update.remove(first_[non_terminal]);
    }

    EraseEmpty(update);

    return update;
  };

  auto curr_added = update_data();
  while (!curr_added.empty()) {
    // merge into result
    for (auto& [non_terminal, terminals] : curr_added) {
      first_[non_terminal].add(terminals);
    }

    // find new data
    curr_added = update_data();
  }
}

void LRTableBuilder::build_states_table() {
  // building goto table
  State start_state;
  for (const auto& production : grammar_.get_start_productions()) {
    start_state.emplace(Position(grammar_.get_start(), production),
                        TokensBitset::only_end());
  }
  start_state = closure(start_state);

  // store State + it's index
  // then all states will be replaced with their indices
  std::vector<decltype(states_)::iterator> updated;

  auto [start_itr, _] = states_.emplace(start_state, StateInfo{0, {}});
  updated.emplace_back(start_itr);

  while (!updated.empty()) {
    auto current_itr = updated.back();
    const auto& [current, info] = *current_itr;
    updated.pop_back();

    // do all possible goto's
    auto grouped = group_by_next(current);

    for (auto& [next_id, state] : grouped) {
      if (next_id == -1) {
        continue;
      }

      state = closure(state);

      auto itr = states_.find(state);
      if (itr == states_.end()) {
        // this is new state, we should process it in the next iterations
        StateInfo new_info{states_.size(), {}};
        std::tie(itr, std::ignore) =
            states_.emplace(std::move(state), std::move(new_info));
        updated.emplace_back(itr);
      }

      current_itr->second.gotos[next_id] = itr->second.index;
    }
  }

  size_t max_nonterm_index =
      std::ranges::max(grammar_.get_productions() | std::views::keys |
                       std::views::transform([](NonTerminal nonterm) {
                         return nonterm.get_id();
                       }));

  goto_.resize(states_.size());
  for (const auto& [index, gotos] : states_ | std::views::values) {
    auto& state_gotos = goto_[index];
    state_gotos.resize(max_nonterm_index + 1);

    for (const auto& [from, to] : gotos) {
      // only non-terminals
      if (from <= -2) {
        goto_[index][-from - 2] = to;
      }
    }
  }
}

void LRTableBuilder::build_actions_table() {
  std::vector<std::vector<std::vector<Action>>> temp_actions;
  temp_actions.resize(states_.size());

  const auto& tokens = Lexis::TokenType::values;

  for (const auto& [state, info] : states_) {
    auto& actions = temp_actions[info.index];
    actions.resize(tokens.size());

    auto grouped = group_by_next(state);

    for (const auto& [position, follow] :
         grouped[GrammarProductionResult::cRuleEndNumber]) {
      Action action;

      if (position.from == grammar_.get_start()) {
        action = AcceptAction{};
      } else {
        size_t remove_count = position.production.first.size();
        NonTerminal next = position.from;
        size_t production_index = position.production.second;

        action = ReduceAction{next, remove_count, production_index};
      }

      for (auto token : tokens) {
        if (follow.contains(token)) {
          actions[static_cast<size_t>(token)].push_back(action);
        }
      }
    }

    for (auto token : tokens) {
      ssize_t index = static_cast<size_t>(token);

      if (grouped.contains(index)) {
        size_t next_state = info.gotos.at(index);
        actions[index].emplace_back(ShiftAction{next_state});
      }

      if (actions[index].empty()) {
        actions[index].emplace_back(RejectAction{});
      }
    }
  }

  std::vector<Conflict> conflicts;
  actions_.resize(states_.size());

  for (size_t state_id = 0; state_id < states_.size(); ++state_id) {
    const auto& state_actions = temp_actions[state_id];
    actions_[state_id].resize(tokens.size());

    for (auto token : tokens) {
      ssize_t index = static_cast<size_t>(token);

      if (state_actions[index].size() != 1) {
        conflicts.emplace_back(state_by_index(state_id)->first, token,
                               state_actions[index]);
        continue;
      }

      actions_[state_id][index] = state_actions[index].front();
    }
  }

  if (!conflicts.empty()) {
    throw ActionsConflictException(std::move(conflicts), std::move(grammar_));
  }
}

State LRTableBuilder::closure(State state) const {
  State result;
  State updated = std::move(state);

  while (!updated.empty()) {
    auto prev_updated = updated;
    for (auto& [position, following] : updated) {
      result[position].add(following);
    }
    updated.clear();

    for (const auto& [updated_position, following] : prev_updated) {
      if (updated_position.iterator == updated_position.end_iterator() ||
          updated_position.iterator.is_terminal()) {
        continue;
      }

      auto non_terminal = updated_position.iterator.access_nonterminal();
      auto following_itr = std::next(updated_position.iterator);

      TokensBitset new_following;
      if (following_itr == updated_position.end_iterator()) {
        // TODO: previous version is commented here
        // theoretically it is wrong, but in practice it worked and lr-table was
        // smaller. Should someday think why it worked this way...
        // new_following = follow_.at(updated_position.from);
        new_following = following;
      } else if (following_itr.is_terminal()) {
        new_following.add(following_itr.access_terminal());
      } else {
        // following_itr is non-terminal
        new_following = first_.at(following_itr.access_nonterminal());
      }

      for (const auto& production :
           grammar_.get_productions_for(non_terminal)) {
        Position new_position(non_terminal, production);

        // meticulously merge new position into resulting state
        updated[new_position].add(new_following);
        updated[new_position].remove(result[new_position]);
      }
    }

    EraseEmpty(updated);
  }

  return result;
}

LRTableBuilder::LRTableBuilder(Grammar grammar) : grammar_(std::move(grammar)) {
  grammar_.check();

  NonTerminal new_start = grammar_.register_nonterm();

  grammar_.add_rule(new_start, grammar_.get_start());
  grammar_.set_start(new_start);

  build_first_table();
  build_states_table();
  build_actions_table();
}

void LRTableBuilder::save_to(const std::filesystem::path& path) const {
  std::ofstream os(path, std::fstream::binary | std::fstream::out);
  LRTableSerializer::serialize(os, actions_, goto_);
}
}  // namespace Syntax

std::ostream& operator<<(std::ostream& os, const Syntax::Action& action) {
  using namespace Syntax;

  if (std::holds_alternative<AcceptAction>(action)) {
    os << "accept";
  } else if (std::holds_alternative<RejectAction>(action)) {
    os << "reject";
  } else if (std::holds_alternative<ShiftAction>(action)) {
    ShiftAction shift = std::get<ShiftAction>(action);
    os << "shift{" << shift.next_state << "}";
  } else {
    ReduceAction reduce = std::get<ReduceAction>(action);
    os << "reduce{" << reduce.next.get_id() << ", " << reduce.remove_count
       << "}";
  }

  return os;
}

std::ostream& operator<<(std::ostream& os, const Syntax::State& state) {
  for (const auto& [position, follow] : state) {
    os << position.to_string() << " [ ";
    for (auto token : Lexis::TokenType::values) {
      if (follow.contains(token)) {
        os << static_cast<size_t>(token) << " ";
      }
    }
    os << "] " << std::endl;
  }

  return os;
}
