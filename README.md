# Интерпретатор для рекурсивных функций Гёделя

## Файлы
Сейчас всё находится в файле `tests/arithmetics.rec`. В будущем, когда я добавлю `#include`, я разнесу
там всё по разным файлам.
В файле `main` находится костыльная, написанная мною сегодня ночью, но хоть какая-то
реализация интерпретатора.
Но лучше в `main` всё-таки не смотреть...

Также `main` пока что не умеет обрабатывать `argmin`, но в файле `secret.rec` есть пример программы,
которая его использует.

## Описание языка
Подробное формальное описание будет подготовлено к следующим итерациям.
Язык очень похож на описание обычных математических функций. Есть немного синтаксического сахара:
1. Проекторы работают под капотом, то есть везде можно использовать просто названия переменных.
2. В рекурсивном вызове функции, чтобы обратиться к предыдущему значению, можно использовать как просто
    название функции (даже без скобок), так и полный синтаксис вызова
   (в этом случае все аргументы должны совпадать с аргументами исходной функции, но в последнем аргументе
    вместо argname+1 должно быть просто argname).
3. В программе можно использовать натуральные числа, при этом `n`, где n - натуральное,
    эквивалентно композиции n функций successor, применённой к функции 0.

## Грамматика
```
program := statement ';' program
         | EMPTY
statement := function_name '(' arguments_list ')' '=' function_name '(' composition_arguments ')'

arguments_list = EMPTY | nonempty_arguments_list
nonempty_arguments_list := variable_name ',' nonempty_arguments_list
                         | variable_name '+' '1'
                         | '0'
                         | variable_name
                
composition_arguments = EMPTY | nonempty_composition_arguments
nonempty_composition_arguments := function_name '(' nonempty_composition_arguments ')' ',' nonempty_composition_arguments
                                | variable_name ',' nonempty_composition_arguments
                                | INTEGER ',' nonempty_composition_arguments
                                | '*' ',' nonempty_composition_arguments
                                | function_name '(' nonempty_composition_arguments ')'
                                | variable_name
                                | INTEGER
                                | '*'
                       
// string is sequence containing only [a-z], [A-Z] and underscore symbols
// function_name and variable_name are string
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
