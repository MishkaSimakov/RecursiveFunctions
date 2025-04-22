#include "SemanticAnalyzer.h"

namespace Front {

bool SemanticAnalyzer::visit_member_expression(MemberExpr& node) {
  Type* type = node.left->type->get_original();

  if (type->get_kind() == Type::Kind::POINTER) {
    scold_user(node, "member access on pointer is forbidden");
  }

  if (type->get_kind() == Type::Kind::STRUCT) {
    // for struct consider member access
    auto& struct_info = std::get<StructSymbolInfo>(
        context_.structs_info.at(&type->as<StructType>()).get());
    SymbolInfo* info = name_lookup(struct_info.subscope, node.member);

    if (info != nullptr && info->is_inside(struct_info.subscope)) {
      auto& member_info = info->as<VariableSymbolInfo>();
      context_.members_info.emplace(&node, *info);

      node.type = member_info.type;
      node.value_category = node.left->value_category;
      return true;
    }
  }

  // look for suitable transformation
  auto* transformation = name_lookup(current_scope_, node.member);
  if (transformation == nullptr) {
    scold_user(node, "no transformation or members with this name found.");
  }

  if (!transformation->is_function()) {
    scold_user(node, "transformation must be a function.");
  }

  FunctionSymbolInfo& transformation_function =
      transformation->as<FunctionSymbolInfo>();
  if (transformation_function.transformation_type != type) {
    if (transformation_function.transformation_type != nullptr) {
      scold_user(node,
                 "function {:?} acts as a transformation on another type: {:?} "
                 "!= {:?}",
                 transformation_function.declaration.name, type,
                 transformation_function.transformation_type);
    } else {
      scold_user(node, "function {:?} is not a transformation",
                 transformation_function.declaration.name);
    }
  }

  context_.members_info.emplace(&node, *transformation);

  node.type = transformation_function.type;
  node.value_category = ValueCategory::LVALUE;

  return true;
}

}  // namespace Front
