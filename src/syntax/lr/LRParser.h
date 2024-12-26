#pragma once

#include <fstream>

#include "LRTableBuilder.h"
#include "LRTableSerializer.h"
#include "lexis/Exceptions.h"
#include "lexis/LexicalAnalyzer.h"
#include "syntax/Exceptions.h"
#include "syntax/grammar/BuildersRegistry.h"

namespace Syntax {
template <typename NodeT>
class LRParser {
  std::vector<std::vector<Action>> actions_;
  std::vector<std::vector<size_t>> goto_;
  const BuildersRegistry<NodeT>& builders_;

 public:
  LRParser(const std::filesystem::path& path,
           const BuildersRegistry<NodeT>& builders)
      : builders_(builders) {
    std::ifstream is(path, std::ios_base::binary);
    std::tie(actions_, goto_) = LRTableSerializer::deserialize(is);
  }

  std::unique_ptr<NodeT> parse(std::span<const Lexing::Token> tokens) const {
    std::vector<size_t> states_stack;

    // use deque because it preserves references
    // TODO: make this memory allocation more efficient
    // maybe implement something like a deque
    std::vector<std::unique_ptr<NodeT>> nodes_stack;

    states_stack.push_back(0);

    size_t index = 0;
    while (index <= tokens.size()) {
      Lexing::Token token = index < tokens.size()
                                ? tokens[index]
                                : Lexing::Token{Lexing::TokenType::END, ""};

      Action action =
          actions_[states_stack.back()][static_cast<size_t>(token.type)];

      if (std::holds_alternative<AcceptAction>(action)) {
        return std::move(nodes_stack.back());
      }
      if (std::holds_alternative<RejectAction>(action)) {
        TokensBitset expected;
        for (auto type : Lexing::TokenType::values) {
          size_t type_id = static_cast<size_t>(type);

          if (!std::holds_alternative<RejectAction>(
                  actions_[states_stack.back()][type_id])) {
            expected.add(type);
          }
        }

        throw UnexpectedTokenException(expected);
      }
      if (std::holds_alternative<ShiftAction>(action)) {
        states_stack.push_back(std::get<ShiftAction>(action).next_state);
        nodes_stack.emplace_back(std::make_unique<NodeT>(token));

        ++index;
      } else {
        // action is reduce
        auto reduce = std::get<ReduceAction>(action);
        auto nodes_span = std::span<std::unique_ptr<NodeT>>{
            nodes_stack.end() - reduce.remove_count, nodes_stack.end()};
        std::unique_ptr<NodeT> new_node =
            builders_[reduce.builder.index](nodes_span);

        states_stack.resize(states_stack.size() - reduce.remove_count);
        nodes_stack.resize(nodes_stack.size() - reduce.remove_count);

        states_stack.push_back(
            goto_[states_stack.back()][reduce.next.get_id()]);
        nodes_stack.push_back(std::move(new_node));
      }
    }

    throw std::runtime_error("Something went terribly wrong.");
  }
};
}  // namespace Syntax
