#include "ASTBuildContext.h"

#include "ASTVisitor.h"

namespace Front {
void ASTBuildContext::set_scope_recursively(ASTNode& node, Scope* scope) {
  class SetScopeVisitor : public ASTVisitor<SetScopeVisitor> {
    Scope* scope_;

  public:
    SetScopeVisitor(ASTNode& root, Scope* scope)
        : ASTVisitor(root), scope_(scope) {}

    NodeTraverseType before_traverse(ASTNode& node) {
      if (node.scope == nullptr) {
        node.scope = scope_;
        return NodeTraverseType::CONTINUE;
      }

      scope_->children.insert(node.scope);
      node.scope->parent = scope_;
      return NodeTraverseType::SKIP_NODE;
    }
  };

  SetScopeVisitor(node, scope).traverse();
}
}
