#include "SemanticAnalyzer.h"

namespace Front {

std::optional<size_t> get_member_index(ClassType* cls, StringId member) {
  for (size_t i = 0; i < cls->members.size(); ++i) {
    if (cls->members[i].first == member) {
      return i;
    }
  }

  return std::nullopt;
}

bool SemanticAnalyzer::visit_member_expression(MemberExpr& node) {
  Type* left_ty = node.left->type;

  if (left_ty->get_kind() == Type::Kind::ALIAS) {
    left_ty = static_cast<AliasType*>(left_ty)->original;
  }

  if (left_ty->get_kind() != Type::Kind::CLASS) {
    scold_user(node, "non-class type {:?} doesn't have members", left_ty);
  }

  ClassType* cls_ty = static_cast<ClassType*>(node.left->type);

  if (node.member.parts.size() > 1) {
    not_implemented("qualified members access");
  }

  auto member_index = get_member_index(cls_ty, node.member.parts.back());
  if (!member_index.has_value()) {
    scold_user(node, "class doesn't have member");
  }

  node.member_index = *member_index;
  node.type = cls_ty->members[*member_index].second;
  node.value_category = node.left->value_category;
  return true;
}

}  // namespace Front
