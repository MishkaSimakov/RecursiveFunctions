#include "ASTInterpreter.h"

bool Interpretation::ASTInterpreter::visit_function_declaration(
    const FunctionDecl& value) {
  if (context_.get_string(value.name) != "main" ||
      !value.parameters.empty() ||
      value.return_type->value->get_kind() != Type::Kind::VOID) {
    throw InterpreterException(
        value, "Only \"main: () -> void\" function is allowed.");
  }

  return true;
}

bool Interpretation::ASTInterpreter::visit_variable_declaration(
    const VariableDecl& value) {
  if (current_nesting_level_ != 0) {
    throw InterpreterException(
        value, "Variable declaration must not appear in nested scope.");
  }

  if (value.type->value->get_kind() != Type::Kind::INT &&
      value.type->value->get_kind() != Type::Kind::BOOL) {
    throw InterpreterException(value,
                               "Only i64 and bool variables are allowed.");
  }

  if (variables_.contains(value.name)) {
    throw InterpreterException(
        value, fmt::format("Redefinition of variable {}.",
                           context_.get_string(value.name)));
  }

  if (value.initializer == nullptr) {
    throw InterpreterException(value, "Value must be assigned to variable.");
  }

  assign_variable(value, values_.at(value.initializer.get()));
  return true;
}

bool Interpretation::ASTInterpreter::traverse_if_statement(
    const IfStmt& value) {
  // in if statement we should not traverse one of the branches

  // calculate condition
  traverse(*value.condition);

  ExpressionValue condition_value = values_.at(value.condition.get());
  if (!std::holds_alternative<bool>(condition_value)) {
    throw InterpreterException(value, "Bool condition must have bool type.");
  }

  ++current_nesting_level_;

  if (std::get<bool>(condition_value)) {
    // evaluate true branch
    traverse(*value.true_branch);
  } else {
    // evaluate false branch
    traverse(*value.false_branch);
  }

  --current_nesting_level_;

  return true;
}

bool Interpretation::ASTInterpreter::traverse_call_expression(
    const CallExpr& value) {
  if (!is_print_function(*value.name)) {
    throw InterpreterException(value, "Unknown function.");
  }

  if (value.arguments.size() != 1) {
    throw InterpreterException(value, "Wrong number of arguments.");
  }

  // evaluate argument
  traverse(*value.arguments.front());

  Expression* first_argument = value.arguments.front().get();
  print_function_implementation(values_.at(first_argument));

  return true;
}

bool Interpretation::ASTInterpreter::visit_id_expression(const IdExpr& value) {
  // we dont allow qualified names
  if (value.parts.size() != 1) {
    throw InterpreterException(value, "Qualified names are not allowed.");
  }

  auto name = value.parts.front();
  if (!variables_.contains(name)) {
    throw InterpreterException(value, "Unknown variable name.");
  }

  values_[&value] = variables_[name];

  return true;
}

bool Interpretation::ASTInterpreter::visit_binary_operator(
    const BinaryOperator& value) {
  using OpType = BinaryOperator::OpType;

  ExpressionValue left = values_[value.left.get()];
  ExpressionValue right = values_[value.right.get()];

  // there are two options:
  // 1. arithmetic + inequalities - both arguments must be i64
  // 2. == and != - types of arguments must be equal (bool or i64)
  if (value.op_type != OpType::NOTEQUAL &&
      value.op_type != OpType::EQUALEQUAL) {
    // all operands must be of int type
    if (!std::holds_alternative<int64_t>(left) ||
        !std::holds_alternative<int64_t>(right)) {
      throw InterpreterException(
          value, "All operands of arithmetic operator must be of i64 type.");
    }

    int64_t left_int = std::get<int64_t>(left);
    int64_t right_int = std::get<int64_t>(right);

    values_[&value] = [&value, left_int, right_int]() -> ExpressionValue {
      switch (value.op_type) {
        case OpType::PLUS:
          return left_int + right_int;
        case OpType::MINUS:
          return left_int - right_int;
        case OpType::MULTIPLY:
          return left_int * right_int;
        case OpType::REMAINDER:
          return left_int % right_int;
        case OpType::LESS:
          return left_int < right_int;
        case OpType::GREATER:
          return left_int > right_int;
        case OpType::LESS_EQ:
          return left_int <= right_int;
        case OpType::GREATER_EQ:
          return left_int >= right_int;
        default:
          unreachable("All arithmetic operators are listed above.");
      }
    }();
  } else {
    if (left.index() != right.index()) {
      throw InterpreterException(value, "Types of arguments must be equal.");
    }

    values_[&value] = std::visit(
        [op_type = value.op_type](auto left, auto right) {
          return op_type == OpType::EQUALEQUAL ? left == right : left != right;
        },
        left, right);
  }

  return true;
}
