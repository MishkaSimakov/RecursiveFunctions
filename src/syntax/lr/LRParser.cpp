#include "LRParser.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <functional>
#include <span>

#include "errors/Helpers.h"

using enum Front::BinaryOperator::OpType;
using namespace Front;
#include "syntax/BuildersRegistry.h"

namespace Syntax {
struct RecoveryTree {
  struct RecoveryNode {
    RecoveryNode* parent;
    std::vector<std::unique_ptr<RecoveryNode>> children;
    SourceRange source_range;
    size_t prev_states_count;
  };

  std::unique_ptr<RecoveryNode> root;
  RecoveryNode* current;

  RecoveryTree(SourceLocation begin) {
    root = std::make_unique<RecoveryNode>();
    root->source_range.begin = begin;

    current = root.get();
  }

  // TODO: this method must be outside of parser
  // returns pointer to node from which just exited
  // nullptr if didn't exit any node
  RecoveryNode* swallow_token(const Lexis::Token& token, size_t states_count) {
    if (token.type == Lexis::TokenType::END) {
      current = nullptr;
      return root.get();
    }

    if (token.type == Lexis::TokenType::OPEN_BRACE) {
      auto& new_node =
          current->children.emplace_back(std::make_unique<RecoveryNode>());
      new_node->parent = current;
      new_node->source_range.begin = token.source_range.begin;
      new_node->prev_states_count = states_count;

      current = new_node.get();
      return nullptr;
    }

    if (token.type == Lexis::TokenType::CLOSE_BRACE) {
      // it is expected that now we can move to parent if there are no syntax
      // errors
      RecoveryNode* old = current;
      current->source_range.end = token.source_range.end;
      current = current->parent;

      // TODO: show this better
      if (current == nullptr) {
        throw std::runtime_error("Parens don't match");
      }

      return old;
    }

    if (token.type == Lexis::TokenType::SEMICOLON) {
    }

    return nullptr;
  }
};

void LRParser::parse(Lexis::LexicalAnalyzer& lexical_analyzer,
                     size_t module_id) const {
  auto& source_manager = context_.source_manager;
  ASTBuildContext build_context(module_id, context_);
  std::vector<size_t> states_stack;

  bool encountered_error = false;

  // TODO: make this memory allocation more efficient
  // maybe implement something like a deque
  std::vector<std::unique_ptr<ASTNode>> nodes_stack;

  states_stack.push_back(0);
  Lexis::Token current_token = lexical_analyzer.get_token();

  RecoveryTree recovery_tree(current_token.source_range.begin);
  recovery_tree.swallow_token(current_token, 1);

  while (true) {
    Action action =
        actions_[states_stack.back()][static_cast<size_t>(current_token.type)];

    if (std::holds_alternative<AcceptAction>(action)) {
      if (!encountered_error) {
        context_.modules[module_id].ast_root = std::unique_ptr<ProgramDecl>(
            dynamic_cast<ProgramDecl*>(nodes_stack.front().release()));
      }

      break;
    }
    if (std::holds_alternative<RejectAction>(action)) {
      encountered_error = true;

      std::vector<std::string_view> expected_tokens;
      for (auto type : Lexis::TokenType::values) {
        size_t type_id = static_cast<size_t>(type);

        if (!std::holds_alternative<RejectAction>(
                actions_[states_stack.back()][type_id])) {
          expected_tokens.push_back(Lexis::TokenType(type).to_string());
        }
      }

      source_manager.add_annotation(
          current_token.source_range.begin,
          fmt::format("Unexpected token {}. Expected: {}",
                      current_token.type.to_string(),
                      fmt::join(expected_tokens, ", ")));

      // try to recover by eliminating whole RecoveryTree branch

      // eliminate code before error
      size_t new_stack_size = recovery_tree.current->prev_states_count;
      states_stack.resize(new_stack_size + 1);
      nodes_stack.clear();

      // eliminate code after error
      auto* eliminated_node = recovery_tree.current;
      while (true) {
        current_token = lexical_analyzer.get_token();
        auto* exited_node =
            recovery_tree.swallow_token(current_token, states_stack.size());

        if (exited_node == eliminated_node) {
          break;
        }
      }

      continue;
    }
    if (std::holds_alternative<ShiftAction>(action)) {
      states_stack.push_back(std::get<ShiftAction>(action).next_state);

      if (!encountered_error) {
        nodes_stack.emplace_back(std::make_unique<TokenNode>(current_token));
      }

      current_token = lexical_analyzer.get_token();
      recovery_tree.swallow_token(current_token, states_stack.size());
    } else {
      // action is reduce
      auto reduce = std::get<ReduceAction>(action);

      if (!encountered_error) {
        auto nodes_span = std::span{nodes_stack.end() - reduce.remove_count,
                                    nodes_stack.end()};

        auto source_range = SourceRange::merge(nodes_span.front()->source_range,
                                               nodes_span.back()->source_range);

        std::unique_ptr<ASTNode> new_node = builders[reduce.production_index](
            &build_context, source_range, nodes_span);

        nodes_stack.resize(nodes_stack.size() - reduce.remove_count);
        nodes_stack.push_back(std::move(new_node));
      }

      states_stack.resize(states_stack.size() - reduce.remove_count);
      states_stack.push_back(goto_[states_stack.back()][reduce.next.get_id()]);
    }
  }

  if (encountered_error) {
    source_manager.print_annotations(std::cout);
    throw std::runtime_error("Syntax errors.");
  }
}
}  // namespace Syntax
