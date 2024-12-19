#pragma once
#include "BuildersRegistry.h"
#include "Grammar.h"

namespace Syntax {
template <typename NodeT, bool SaveGrammar>
class GrammarBuilder {
  struct Empty {};

 public:
  BuildersRegistry<NodeT> builders;
  std::conditional_t<SaveGrammar, Grammar, Empty> grammar;

  void add(NonTerminal from, GrammarProductionResult to,
           typename BuildersRegistry<NodeT>::BuilderFnT builder_fn) {
    auto builder = builders.register_builder(std::move(builder_fn));

    if constexpr (SaveGrammar) {
      grammar.add_rule(from, std::move(to), builder);
    }
  }

  void set_start(NonTerminal start) {
    if constexpr (SaveGrammar) {
      grammar.set_start(start);
    }
  }

  NonTerminal nonterm() {
    if constexpr (SaveGrammar) {
      return grammar.register_nonterm();
    } else {
      return NonTerminal{0};
    }
  }
};
}  // namespace Syntax
