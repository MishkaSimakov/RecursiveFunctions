#include "LRParser.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <span>

using enum Front::BinaryOperator::OpType;
using namespace Front;
#include "syntax/BuildersRegistry.h"

namespace Syntax {

// Recovery tree helps to recover from syntax errors.
// When LRParser encounters error some part of program must be removed to
// continue execution. Part of program before error is removed using
// `prev_states_count` value. Part after error is removed using `prune_subtree`
// method.
// Order: when LRParser process token, RecoveryTree has already processed it.
// In some cases recovery algorithm can brake. Then parser would not recover
// from error and just crash.
class RecoveryTree {
 public:
  enum class NodeType { ROOT, BRACE, SEMICOLON };

  struct RecoveryNode {
    size_t prev_states_count;
    NodeType type;

    RecoveryNode(size_t prev_states_count, NodeType type)
        : prev_states_count(prev_states_count), type(type) {}
  };

 private:
  std::vector<RecoveryNode> nodes_;
  bool is_broken_ = false;

 public:
  RecoveryTree() { nodes_.emplace_back(1, NodeType::ROOT); }

  const RecoveryNode& get_current_node() const { return nodes_.back(); }

  // TODO: this method must be outside of parser
  void swallow_token(Lexis::TokenType token, size_t states_count) {
    if (is_broken_) {
      return;
    }

    if (token == Lexis::TokenType::OPEN_BRACE) {
      nodes_.emplace_back(states_count, NodeType::BRACE);
    } else if (token == Lexis::TokenType::CLOSE_BRACE) {
      nodes_.pop_back();

      if (nodes_.empty()) {
        // parens are not balanced => our recovery algorithm doesn't work :(
        is_broken_ = true;
        return;
      }
    } else if (token == Lexis::TokenType::SEMICOLON) {
      // defered_ = NodeType::SEMICOLON;
    }
  }

  bool is_broken() const { return is_broken_; }

  void prune_subtree(Lexis::LexicalAnalyzer& lexical_analyzer) {
    size_t prune_point = nodes_.size();
    Lexis::TokenType token_type = lexical_analyzer.current_token().type;

    // read tokens until we reach prune_point parent
    while (true) {
      if (token_type == Lexis::TokenType::END) {
        return;
      }

      if (nodes_.size() == prune_point) {
        // when we prune subtree with braces we should leave CLOSE_BRACE token:
        // before: f: () -> void = { error! }
        // after:  f: () -> void = {        }
        if (token_type == Lexis::TokenType::CLOSE_BRACE) {
          return;
        }

        // but with semicolon we should skip SEMICOLON token too
        // before: f: () -> void = { error!; call(); }
        // after:  f: () -> void = {         call(); }
        if (token_type == Lexis::TokenType::SEMICOLON) {
          lexical_analyzer.next_token();
          return;
        }
      }

      // states count here doesn't matter, all nodes, created here, will be
      // skipped
      swallow_token(token_type, 0);

      // shift lexical_analyzer to next token
      token_type = lexical_analyzer.next_token().type;
    }
  }
};

void LRParser::parse(Lexis::LexicalAnalyzer& lexical_analyzer,
                     ModuleContext& module_context) const {
  ASTBuildContext build_context(module_id, context_);
  std::vector<size_t> states_stack;

  std::vector<std::pair<SourceRange, std::string>> errors;

  // TODO: make this memory allocation more efficient
  // maybe implement something like a deque
  std::vector<std::unique_ptr<ASTNode>> nodes_stack;

  states_stack.push_back(0);
  Lexis::Token current_token = lexical_analyzer.next_token();

  RecoveryTree recovery_tree;

  while (true) {
    Action action =
        actions_[states_stack.back()][static_cast<size_t>(current_token.type)];

    if (std::holds_alternative<AcceptAction>(action)) {
      if (errors.empty()) {
        context_.modules[module_id].ast_root = std::unique_ptr<ProgramDecl>(
            dynamic_cast<ProgramDecl*>(nodes_stack.front().release()));
      }

      break;
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

      errors.emplace_back(current_token.source_range,
                          fmt::format("Unexpected token {}. Expected: {}",
                                      current_token.type.to_string(),
                                      fmt::join(expected_tokens, ", ")));

      // try to recover using RecoveryTree
      // if it is broken then there is nothing we can do
      if (recovery_tree.is_broken()) {
        break;
      }

      // eliminate code before error
      const auto& eliminated_node = recovery_tree.get_current_node();
      size_t new_stack_size = eliminated_node.prev_states_count;
      states_stack.resize(new_stack_size);
      nodes_stack.clear();

      // eliminate code after error
      recovery_tree.prune_subtree(lexical_analyzer);

      // RecoveryTree can brake after skipping some tokens.
      // Therefore we have to check again.
      if (recovery_tree.is_broken()) {
        break;
      }

      current_token = lexical_analyzer.current_token();

      continue;
    }
    if (std::holds_alternative<ShiftAction>(action)) {
      states_stack.push_back(std::get<ShiftAction>(action).next_state);

      if (errors.empty()) {
        nodes_stack.emplace_back(std::make_unique<TokenNode>(current_token));
      }

      recovery_tree.swallow_token(current_token.type, states_stack.size());
      current_token = lexical_analyzer.next_token();
    } else {
      // action is reduce
      auto reduce = std::get<ReduceAction>(action);

      if (errors.empty()) {
        auto nodes_span = std::span{nodes_stack.end() - reduce.remove_count,
                                    nodes_stack.end()};

        SourceRange source_range =
            nodes_span.empty()
                ? SourceRange::empty_at(current_token.source_range.begin)
                : SourceRange::merge(nodes_span.front()->source_range,
                                     nodes_span.back()->source_range);

        std::unique_ptr<ASTNode> new_node =
            (build_context.*builders[reduce.production_index])(source_range,
                                                               nodes_span);

        nodes_stack.resize(nodes_stack.size() - reduce.remove_count);
        nodes_stack.push_back(std::move(new_node));
      }

      states_stack.resize(states_stack.size() - reduce.remove_count);
      states_stack.push_back(goto_[states_stack.back()][reduce.next.get_id()]);
    }
  }

  if (!errors.empty()) {
    throw ParserException(std::move(errors));
  }
}
}  // namespace Syntax
