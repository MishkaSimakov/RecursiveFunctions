#include "IRASTVisitor.h"

bool Front::IRASTVisitor::traverse_function_declaration(
    const FunctionDecl& value) {
  IR::Function function(value.name);

  // create arguments
  for (auto param : value.parameters) {
    IR::Type* type = map_type(param->type->value);
    IR::Temporary* temp = function.add_temporary(type);

    function_symbols_mapping_.emplace(std::pair{param->id, param->scope}, temp);

    function.arguments.push_back(temp);
  }

  function.return_type = map_type(value.return_type->value);

  auto& func = program_.functions.emplace_back(std::move(function));
  auto begin_block = func.add_block();
  func.begin_block = begin_block;

  current_basic_block_ = begin_block;

  // extra careful here lest emplace new function while traversing
  current_function_ = &func;
  function_symbols_mapping_.clear();

  traverse(*value.body);

  return true;
}
