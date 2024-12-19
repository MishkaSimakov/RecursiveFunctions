#pragma once

#include "syntax/SyntaxNode.h"
#include "syntax/grammar/GrammarBuilder.h"

namespace Syntax {
#ifdef COMPILING_GRAMMAR
constexpr bool cIsCompilingGrammar = true;
#else
constexpr bool cIsCompilingGrammar = false;
#endif

extern GrammarBuilder<SyntaxNode, cIsCompilingGrammar>
get_recursive_functions_grammar();
}  // namespace Syntax
