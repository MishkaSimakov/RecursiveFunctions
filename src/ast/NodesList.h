// some nodes are only available during AST construction phase
// they are not present in this list

#ifndef NODE
#define NODE(enum_case, type, snake_case)
#endif

NODE(PROGRAM, ProgramNode, program);

NODE(FUNCTION_DECL, FunctionDecl, function_declaration);
NODE(IMPORT_DECL, ImportDecl, import_declaration);
NODE(VARIABLE_DECL, VariableDecl, variable_declaration);
NODE(NAMESPACE_DECL, NamespaceDecl, namespace_declaration);
NODE(TYPE_ALIAS_DECL, TypeAliasDecl, type_alias_declaration);
NODE(CLASS_DECL, ClassDecl, class_declaration);

NODE(WHILE_STMT, WhileStmt, while_statement);
NODE(IF_STMT, IfStmt, if_statement);
NODE(BREAK_STMT, BreakStmt, break_statement);
NODE(CONTINUE_STMT, ContinueStmt, continue_statement);
NODE(COMPOUND_STMT, CompoundStmt, compound_statement);
NODE(RETURN_STMT, ReturnStmt, return_statement);
NODE(ASSIGNMENT_STMT, AssignmentStmt, assignment_statement);
NODE(DECLARATION_STMT, DeclarationStmt, declaration_statement);
NODE(EXPRESSION_STMT, ExpressionStmt, expression_statement);

NODE(INTEGER_LITERAL_EXPR, IntegerLiteral, integer_literal);
NODE(STRING_LITERAL_EXPR, StringLiteral, string_literal);
NODE(BOOL_LITERAL_EXPR, BoolLiteral, bool_literal);
NODE(ID_EXPR, IdExpr, id_expression);
NODE(BINARY_OPERATOR_EXPR, BinaryOperator, binary_operator);
NODE(UNARY_OPERATOR_EXPR, UnaryOperator, unary_operator);
NODE(CALL_EXPR, CallExpr, call_expression);
NODE(MEMBER_EXPR, MemberExpr, member_expression);
NODE(TUPLE_EXPR, TupleExpr, tuple_expression);
NODE(IMPLICIT_LVALUE_TO_RVALUE_CONVERSION_EXPR, ImplicitLvalueToRvalueConversionExpr, implicit_lvalue_to_rvalue_conversion_expression);
NODE(TUPLE_INDEX_EXPR, TupleIndexExpr, tuple_index_expression);

NODE(POINTER_TYPE, PointerTypeNode, pointer_type);
NODE(PRIMITIVE_TYPE, PrimitiveTypeNode, primitive_type);
NODE(TUPLE_TYPE, TupleTypeNode, tuple_type);
NODE(USER_DEFINED_TYPE, UserDefinedTypeNode, user_defined_type);