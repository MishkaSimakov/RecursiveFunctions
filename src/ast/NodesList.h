// some nodes are only available during AST construction phase
// they are not present in this list

#ifndef NODE
#define NODE(enum_case, type, snake_case)
#endif

NODE(INT_TYPE, IntType, int_type);
NODE(BOOL_TYPE, BoolType, bool_type);

NODE(COMPOUND_STMT, CompoundStmt, compound_statement);
NODE(PROGRAM_DECL, ProgramDecl, program_declaration);
NODE(PARAMETER_DECL, ParameterDecl, parameter_declaration);
NODE(FUNCTION_DECL, FunctionDecl, function_declaration);
NODE(RETURN_STMT, ReturnStmt, return_statement);
NODE(INTEGER_LITERAL_EXPR, IntegerLiteral, integer_literal);
NODE(STRING_LITERAL_EXPR, StringLiteral, string_literal);
NODE(ID_EXPR, IdExpr, id_expression);
NODE(IMPORT_DECL, ImportDecl, import_declaration);
NODE(BINARY_OPERATOR, BinaryOperator, binary_operator);
NODE(CALL_EXPR, CallExpr, call_expression);
