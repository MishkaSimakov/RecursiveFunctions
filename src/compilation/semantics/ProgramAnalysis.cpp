#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::before_program_declaration(ProgramDecl& node) {
  // root scope has no parent
  auto scope = std::make_unique<Scope>();

  current_scope_ = scope.get();
  context_.root_scope = std::move(scope);

  return true;
}

bool SemanticAnalyzer::after_program_declaration(ProgramDecl& node) {
  return true;
}
}  // namespace Front
