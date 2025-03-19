// some nodes are only available during AST construction phase
// they are not present in this list

#ifndef NODE
#define NODE(enum_case, type, snake_case)
#endif

NODE(COMPOUND_STMT, CompoundStmt, compound_statement);
NODE(PROGRAM_DECL, ProgramDecl, program_declaration);
NODE(PARAMETER_DECL, ParameterDecl, parameter_declaration);
NODE(FUNCTION_DECL, FunctionDecl, function_declaration);
NODE(RETURN_STMT, ReturnStmt, return_statement);
NODE(INTEGER_LITERAL_EXPR, IntegerLiteral, integer_literal);
NODE(STRING_LITERAL_EXPR, StringLiteral, string_literal);
NODE(BOOL_LITERAL_EXPR, BoolLiteral, bool_literal);
NODE(ID_EXPR, IdExpr, id_expression);
NODE(IMPORT_DECL, ImportDecl, import_declaration);
NODE(BINARY_OPERATOR_EXPR, BinaryOperator, binary_operator);
NODE(UNARY_OPERATOR_EXPR, UnaryOperator, unary_operator);
NODE(CALL_EXPR, CallExpr, call_expression);
NODE(TYPE_NODE, TypeNode, type_node);
NODE(VARIABLE_DECL, VariableDecl, variable_declaration);
NODE(ASSIGNMENT_STMT, AssignmentStmt, assignment_statement);
NODE(DECLARATION_STMT, DeclarationStmt, declaration_statement);
NODE(EXPRESSION_STMT, ExpressionStmt, expression_statement);
NODE(NAMESPACE_DECL, NamespaceDecl, namespace_declaration);
NODE(WHILE_STMT, WhileStmt, while_statement);
NODE(IF_STMT, IfStmt, if_statement);
NODE(BREAK_STMT, BreakStmt, break_statement);
NODE(CONTINUE_STMT, ContinueStmt, continue_statement);
