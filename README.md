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

## Выполнение программы состоит из следующих шагов
1. Препроцессинг: на этом этапе убираются все комментарии, вместо #include подставляются необходимые части других файлов.
   После этого этапа вся программа представляет из себя одну большую строку.
2. Нахождение токенов: вся эта большая строка разбивается на маленькие части - токены.
3. Построение дерева разбора: для массива из токенов строится дерево разбора.
   Оно показывает, какими действиями данный код был получен с помощью рекурсивных правил грамматики.
   Для этого используется простой и не очень оптимальный алгоритм, который пытается съесть как можно больше кода,
   но жадность часто приводит его не туда, и ему приходится откатываться назад.
4. Построение абстрактного синтаксического дерева: по дереву разбора строится абстрактное синтаксическое дерево.
   Оно содержит в себе меньше лишних деталей, связанных с текстовым представлением программы.
5. Компиляция: на этом этапе абстрактное синтаксическое дерево превращается в байт-код - специальный язык для
   моей стековой виртуальной машины.
6. Исполнение байт-кода: виртуальная машина исполняет байт-код.

Файлы, ответственные за каждый из этапов лежат в соответствующих папках.