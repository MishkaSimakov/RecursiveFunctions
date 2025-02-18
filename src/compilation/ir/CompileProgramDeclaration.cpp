#include "IRASTVisitor.h"

bool Front::IRASTVisitor::traverse_program_declaration(
    const ProgramDecl& value) {
  for (auto& decl : value.declarations) {
    traverse(*decl);
  }

  return true;
}
