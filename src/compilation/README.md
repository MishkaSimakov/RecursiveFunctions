# Компиляция в байт-код

Перед выполнением абстрактное синтаксическое дерево должно быть переведено в байт-код.
Он будет состоять из инструкций для стековой машины.

Этапы преобразования в байт-код:
1. Преобразовать имена и названия функций в числа
2. Рекурсивно сгенерировать байт-код для каждой функции
3. Склеить все вызовы в один файл
4. Произвести оптимизации байт-кода (опционально) или обфускацию
    может быть кто-нибудь захочет проприетарное ПО на рекурсивных функциях написать

Исполнитель состоит из двух стеков.
1. Стек вызовов функций и их аргументов (стек вызовов)
2. Стек расчёта значений, на котором выполняются операции (стек вычислений)

## Команды байт-кода
1.  INCREMENT n - увеличить на 1 значение в n-й с конца ячейке стека вычислений.
2.  DECREMENT n - уменьшить на 1 значение в n-й с конца ячейке стека вычислений.
3.  POP_JUMP_IF_ZERO n - убрать последнее значение со стека вычислений,
    если оно 0 - прыгнуть на строку с номером n,
    иначе - продолжить исполнение со следующей строки.
4.  JUMP_IF_NONZERO n - посмотреть на последнее значение в стеке вычислений,
    если 0 - продолжить исполнение со следующей строки,
    иначе - прыгнуть на строку с номером n.
5.  CALL_FUNCTION - запускает процедуру вызова функции. Процедура такова:
    На стек вызовов переносятся последовательно все значения из стека вычислений до тех пор,
    пока не встретится значение, положенное в него инструкцией LOAD_CALL.
    В этот момент из стека вычислений убирается это значение, но не кладётся на стек вызовов,
    исполнение переходит на строчку с данным номером.
6.  LOAD n - кладёт n-й с конца элемент в стеке вызовов наверх стека вычислений.
7.  LOAD_CONST n - кладёт число n на стек вызовов.
8.  LOAD_CALL n - кладёт на стек номер строчки для вызова функции (исполнитель байт-кода умеет отличать
9.  COPY n - берёт n-й с конца элемент в стеке вычислений и кладёт его наверх стека вычислений.
    обычное значение на стеке от номера строчки для вызова функции)
10. RETURN - возвращает исполнение предыдущей функции, на стеке вычислений остаётся последнее значение на нём
    (то есть функция возвращает то, что лежит в конце этого стека в момент вызова RETURN)
11. POP n - убирает n-й с конца элемент со стека вычислений
12. HALT - заканчивает исполнение программы. Последнее значение в стеке вычислений является результатом работы
    алгоритма.

## Пример байт-кода для рекурсивной функции add:
ADD:
1. LOAD 0
2. JUMP_IF_ZERO 8
3. DECREMENT 0
4. LOAD 1
5. CALL ADD
6. INCREMENT 0
7. RETURN
8. POP
9. PUSH_CONST 0
10. RETURN

При этом вычисление 2 + 2 будет выглядеть так:
1. LOAD_CONST 2
2. LOAD_CONST 2
3. CALL ADD
4. HALT

Здесь в вызовах CALL слово ADD будет заменено на номер функции.
Номер будет кодировать положение кода функции в общем файле.