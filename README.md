# Интерпретатор для рекурсивных функций Гёделя

## Файлы
Сейчас всё находится в файле `tests/arithmetics.rec`. В будущем, когда я добавлю `#include`, я разнесу
там всё по разным файлам.
В файле `main` находится костыльная, написанная мною сегодня ночью, но хоть какая-то
реализация интерпретатора.
Но лучше в `main` всё-таки не смотреть...

Также `main` пока что не умеет обрабатывать `argmin`, но в файле `secret.rec` есть пример программы,
которая его использует.

## Грамматика
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

## Идея работы интерпретатора
Перед запуском программы будет выполняться препроцессинг кода.
Он будет предельно прост:
1. Убираем # "comment"
2. Заменяем во всех файлах #include "filename" на текст самого файла.
    (надо будет дерево построить и идти, начиная с листьев)
3. Убрать все пробелы и переносы строк

На выходе получаем однострочный файл со всем кодом.
Он подаётся на вход программе, которая строит дерево разбора всего этого кода.
