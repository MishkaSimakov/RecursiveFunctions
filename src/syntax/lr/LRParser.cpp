#include "LRParser.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <functional>
#include <span>

#include "errors/Helpers.h"

using enum BinaryOperator::OpType;
#include "syntax/BuildersRegistry.h"

namespace Syntax {
ASTContext LRParser::parse(Lexis::LexicalAnalyzer& lexical_analyzer,
                           ASTBuildContext& build_context) const {
  std::vector<size_t> states_stack;

  // use deque because it preserves references
  // TODO: make this memory allocation more efficient
  // maybe implement something like a deque
  std::vector<std::unique_ptr<ASTNode>> nodes_stack;

  states_stack.push_back(0);
  Lexis::Token current_token = lexical_analyzer.get_token();

  while (true) {
    Action action =
        actions_[states_stack.back()][static_cast<size_t>(current_token.type)];

    if (std::holds_alternative<AcceptAction>(action)) {
      return build_context.move_context(std::move(nodes_stack.front()));
    }
    if (std::holds_alternative<RejectAction>(action)) {
      std::vector<std::string_view> expected_tokens;
      for (auto type : Lexis::TokenType::values) {
        size_t type_id = static_cast<size_t>(type);

        if (!std::holds_alternative<RejectAction>(
                actions_[states_stack.back()][type_id])) {
          expected_tokens.push_back(Lexis::TokenType(type).to_string());
        }
      }

      source_manager_.add_annotation(
          current_token.source_range.begin,
          fmt::format("Unexpected token {}. Expected: {}",
                      current_token.type.to_string(),
                      fmt::join(expected_tokens, ", ")));
      source_manager_.print_annotations(std::cout);
      throw std::runtime_error("Unexpected token.");
    }
    if (std::holds_alternative<ShiftAction>(action)) {
      states_stack.push_back(std::get<ShiftAction>(action).next_state);
      nodes_stack.emplace_back(std::make_unique<TokenNode>(current_token));

      current_token = lexical_analyzer.get_token();
    } else {
      // action is reduce
      auto reduce = std::get<ReduceAction>(action);
      auto nodes_span =
          std::span{nodes_stack.end() - reduce.remove_count, nodes_stack.end()};

      auto source_range = SourceRange::merge(nodes_span.front()->source_range(),
                                             nodes_span.back()->source_range());

      std::unique_ptr<ASTNode> new_node = builders[reduce.production_index](
          &build_context, source_range, nodes_span);

      states_stack.resize(states_stack.size() - reduce.remove_count);
      nodes_stack.resize(nodes_stack.size() - reduce.remove_count);

      states_stack.push_back(goto_[states_stack.back()][reduce.next.get_id()]);
      nodes_stack.push_back(std::move(new_node));
    }
  }

  unreachable("Parser must either reject or accept string.");
}
}  // namespace Syntax
