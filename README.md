# Интерпретатор для рекурсивных функций Гёделя

## Грамматика (пока что устарела)
```
program := statement ';' program
         | EMPTY
statement := FUNCTION_NAME '(' arguments_list ')' '=' 
        (FUNCTION_NAME '(' composition_arguments ')' | INTEGER | IDENTIFIER)

arguments_list = EMPTY | nonempty_arguments_list
nonempty_arguments_list := VARIABLE_NAME ',' nonempty_arguments_list
                         | VARIABLE_NAME '+' '1'
                         | '0'
                         | VARIABLE_NAME
                
composition_arguments = EMPTY | nonempty_composition_arguments
nonempty_composition_arguments := FUNCTION_NAME '(' nonempty_composition_arguments ')' ',' nonempty_composition_arguments
                                | VARIABLE_NAME ',' nonempty_composition_arguments
                                | INTEGER ',' nonempty_composition_arguments
                                | '*' ',' nonempty_composition_arguments
                                | FUNCTION_NAME '(' nonempty_composition_arguments ')'
                                | VARIABLE_NAME
                                | INTEGER
                                | '*'
                       
// string is sequence containing only [a-z], [A-Z] and underscore symbols
// FUNCTION_NAME and VARIABLE_NAME are strings
// FUNCTION_NAME, VARIABLE_NAME, INTEGER, EMPTY and all symbols in '' are terminating characters.
```