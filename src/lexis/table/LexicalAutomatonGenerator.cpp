#include "LexicalAutomatonGenerator.h"

#include <fmt/format.h>

#include <fstream>
#include <ranges>
#include <vector>

#include "LexicalTableSerializer.h"
#include "lexis/automata/FiniteAutomata.h"
#include "lexis/regex/CustomRegex.h"
#include "lexis/table/LexicalAutomatonState.h"

namespace Lexis {
static void ReplaceAll(std::string& str, const std::string& from,
                       const std::string& to) {
  if (from.empty()) {
    return;
  }

  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

static std::optional<FinishJump> CanCompactNode(const JumpTableT& jumps) {
  if (!std::holds_alternative<FinishJump>(jumps[0])) {
    return {};
  }

  for (const auto& action : jumps | std::views::drop(1)) {
    if (action != jumps[0]) {
      return {};
    }
  }

  auto finish_jump = std::get<FinishJump>(jumps[0]);
  if (finish_jump.forward_shift == 0) {
    return {};
  }

  return finish_jump;
}

void LexicalAutomatonGenerator::build_and_save(
    const std::filesystem::path& save_path) {
  constexpr size_t kTokensCount = TokenType::count;

  // replace helpers with their value and build automatons for them
  std::array<FiniteAutomata, kTokensCount> tokens_automata;

  for (auto& [token, value] : tokens_) {
    for (const auto& [helper, replacement] : helpers_) {
      ReplaceAll(value, "{" + helper + "}", "(" + replacement + ")");
    }

    auto automaton = FiniteAutomata(Regex(value)).get_minimal();
    automaton.remove_dead_ends();

    tokens_automata[static_cast<size_t>(token)] = std::move(automaton);
  }

  // merge automata
  // create mapping from new states to vector of old ones
  std::unordered_map<StatesMappingT, size_t, decltype(states_hasher_fn)>
      new_states;
  std::vector<JumpTableT> jumps;

  // create first state
  new_states[StatesMappingT{}] = 0;
  jumps.emplace_back();

  // calculate jumps
  std::vector queue = {new_states.begin()};

  while (!queue.empty()) {
    const auto& [mapping, index] = *queue.back();
    queue.pop_back();

    std::optional<size_t> final_token;
    for (size_t i = 0; i < kTokensCount; ++i) {
      if (mapping[i] != -1 && tokens_automata[i].nodes[mapping[i]].is_final) {
        if (final_token.has_value()) {
          // two final states overlap => fail
          throw std::runtime_error(fmt::format(
              "Failed to build automaton. Tokens {} and {} match same "
              "sequence.",
              TokenType(i).toString(),
              TokenType(final_token.value()).toString()));
        }

        final_token = i;
      }
    }

    bool has_final_jump = false;
    for (size_t symbol = 0; symbol < Charset::kCharactersCount; ++symbol) {
      StatesMappingT next_state{};

      for (size_t token = 0; token < kTokensCount; ++token) {
        next_state[token] =
            mapping[token] != -1
                ? tokens_automata[token].nodes[mapping[token]].jumps[symbol]
                : -1;
      }

      // if there is at least one jump then it is NextStateJump
      if (std::ranges::any_of(next_state,
                              [](ssize_t value) { return value != -1; })) {
        auto [itr, was_emplaced] =
            new_states.emplace(next_state, new_states.size());

        if (was_emplaced) {
          jumps.emplace_back();
          queue.push_back(itr);
        }

        jumps[index][symbol] = NextStateJump{itr->second};

        continue;
      }

      // else it is Reject or Finish
      if (final_token.has_value()) {
        has_final_jump = true;
        jumps[index][symbol] = FinishJump{1, TokenType(final_token.value())};
      } else {
        jumps[index][symbol] = RejectJump{};
      }
    }

    if (final_token.has_value() && !has_final_jump) {
      throw std::runtime_error(
          fmt::format("Token {} is unrecognizable.",
                      TokenType(final_token.value()).toString()));
    }
  }

  // now we can compact some jumps
  std::vector<JumpTableT> compacted;
  std::vector<JumpT> mapping(jumps.size());

  for (size_t i = 0; i < jumps.size(); ++i) {
    auto value = CanCompactNode(jumps[i]);

    if (!value.has_value()) {
      mapping[i] = NextStateJump{compacted.size()};
      compacted.push_back(jumps[i]);

      continue;
    }

    auto finish_jump = value.value();
    --finish_jump.forward_shift;
    mapping[i] = finish_jump;
  }

  for (auto& node : compacted) {
    for (auto& jump : node) {
      if (!std::holds_alternative<NextStateJump>(jump)) {
        continue;
      }

      size_t next_node = std::get<NextStateJump>(jump).state_id;
      jump = mapping[next_node];
    }
  }

  std::ofstream os(save_path);

  if (!os) {
    throw std::runtime_error("Failed to open file.");
  }

  LexicalTableSerializer::serialize(os, compacted);
}
}  // namespace Lexis
