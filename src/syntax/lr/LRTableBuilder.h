#pragma once
#include <filesystem>

#include "Position.h"
#include "TokensBitset.h"
#include "lexis/LexicalAnalyzer.h"
#include "syntax/grammar/Grammar.h"

namespace Syntax {
using State = std::unordered_map<Position, TokensBitset>;

struct StateInfo {
  size_t index;
  std::unordered_map<ssize_t, size_t> gotos;
};

struct RejectAction {};
struct AcceptAction {};
struct ReduceAction {
  NonTerminal next;
  size_t remove_count;
  size_t production_index;
};
struct ShiftAction {
  size_t next_state;
};

using Action =
    std::variant<RejectAction, AcceptAction, ReduceAction, ShiftAction>;

struct Conflict {
  State state;
  char symbol;
  std::vector<Action> actions;

  Conflict(State state, char symbol, std::vector<Action> actions)
      : state(std::move(state)), symbol(symbol), actions(std::move(actions)) {}
};

struct ActionsConflictException : std::exception {
  std::vector<Conflict> conflicts;
  Grammar grammar;

  explicit ActionsConflictException(std::vector<Conflict> conflicts,
                                    Grammar grammar)
      : conflicts(std::move(conflicts)), grammar(std::move(grammar)) {}

  const char* what() const noexcept override {
    return "conflicting actions in LRTableBuilder";
  }
};

class LRTableBuilder {
  struct StatesHasher {
    size_t operator()(const State& state) const {
      return unordered_range_hasher_fn(
          state | std::views::transform([](const State::value_type& element) {
            return tuple_hasher_fn(element.first, element.second);
          }));
    }
  };

  Grammar grammar_;

  std::unordered_map<NonTerminal, TokensBitset> first_;
  std::unordered_map<NonTerminal, TokensBitset> follow_;
  std::unordered_map<State, StateInfo, StatesHasher> states_;
  std::vector<std::vector<size_t>> goto_;
  std::vector<std::vector<Action>> actions_;

  static std::unordered_map<ssize_t, State> group_by_next(const State& state);

  void build_first_table();
  void build_follow_table();
  void build_states_table();
  void build_actions_table();

  State closure(State state) const;

  auto state_by_index(size_t index) const {
    return std::ranges::find_if(states_, [index](const auto& pair) {
      return pair.second.index == index;
    });
  }

 public:
  using ActionsTableT = decltype(actions_);
  using GotoTableT = decltype(goto_);

  explicit LRTableBuilder(Grammar grammar);

  auto& get_actions_table() { return actions_; }
  auto& get_first_table() { return first_; }
  auto& get_follow_table() { return follow_; }
  auto& get_goto_table() { return goto_; }

  void save_to(const std::filesystem::path& path) const;
};
}  // namespace Syntax

std::ostream& operator<<(std::ostream& os, const Syntax::Action& action);

std::ostream& operator<<(std::ostream& os, const Syntax::State& state);
