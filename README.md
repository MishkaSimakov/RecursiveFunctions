# Интерпретатор для рекурсивных функций Гёделя

Набор инструментов для исполнения и отладки рекурсивных функций Гёделя.

## Содержание

- [Кто такие эти функции?](#functions)
- [Язык для описания программ]
- [Запуск программ]
- Примеры
- Детали работы интерпретатора
    - Pipeline интерпретатора
    - Препроцессор
    - Лексический анализ
    - Синтаксический анализ
    - Построение дерева компиляции
    - Компиляция в байткод
    - Исполнение байткода

<a name="functions">
## Кто такие эти функции?
</a>

Этот интерпретатор позволяет исполнять рекурсивные функции Гёделя.
Это одна из конструкций, позволяющая анализировать программы и их свойства.
Писать код на них неудобно, но интересно. Они позволяют по-новому взглянуть на знакомые всем вещи, такие как циклы,
функции, условные операторы.
В рекурсивных функциях, как это следует из названия, есть лишь рекурсия и функции - никаких циклов и if-ов.
Можно объявлять функции и подавать на вход функциям другие функции. Чтобы не было слишком печально, у вас по умолчанию
есть функция +1 (она принимает одно число и возвращает следующее за ним), а также все константы (0, 1, 2, ...).

Но у внимательного читателя сразу возникнет вопрос - а как же мне, простому работяге, реализовать функцию Аккермана???
Хмм, действительно, функцию Аккермана (если вы вдруг не знаете, что это, то быстрее исправьте это
недоразумение: https://ru.wikipedia.org/wiki/Функция_Аккермана) нам не реализовать в такой постановке.

Поэтому мы добавляем ещё одну супер-мега функцию. Она принимает другую функцию и возвращает минимальное значение
аргумента, на котором она равна 0. С таким дополнением мы можем написать уже почти все адекватные программы (программу,
различающую язык HALT адекватной мы не считаем).

Подробнее можно почитать здесь: https://en.wikipedia.org/wiki/General_recursive_function

## Язык для описания программ

Для описания рекурсивных функций используется специальный язык программирования, похожий на почти все основные языки.
В нём есть две основные сущности: функции и натуральные числа (0, 1, 2, ...). Все функции принимают на вход и возвращают
лишь натуральные числа.

### Обычные функции

Пример объявления обычной функции:

```
f(x, y, z) = g(t(x), d(y), z);
```

Этот код означает, что функция `f` принимает 3 числа на вход (`x`, `y` и `z`). Она вызывает функции `t` и `d`,
подставляя в них `x` и `y` соответственно. После этого она вызывает `g`, подставляя в неё значения, вычисленные
функциями `t` и `d`, а в качестве третьего аргумента передаётся `z`.

Все обычные функции должны быть определены при объявлении. Также на момент объявления все функции, используемые в
определении, должны быть объявлены (и определены).

### Рекурсивные функции

Объявление рекурсивной функции похоже на объявление обычной, но оно состоит из двух частей. Для примера покажем, как
написать функцию `add(x, y)`, которая возвращает результат сложения `x` и `y`.

```
# начальный случай
add(x, 0) = x;

# общий случай
add(x, y + 1) = successor(add);
```

Здесь сразу много нового:

1. Строки, начинающиеся с символа `#` - это комментарии. Они полностью игнорируются при исполнении.
2. successor - это встроенная функция - её не нужно определять. Она принимает на вход одно число и возвращает следующее
   за ним.
3. Начальный случай: он выполняется, если при вызове функции `add` последний аргумент равен 0. В нашем
   случае: `x + 0 = x`.
4. Общий случай: он выполняется, если последний аргумент не равен 0. При этом переменная `y` здесь - это параметр
   рекурсии. На это указывает специальный синтаксис: `y + 1`. Параметр рекурсии всегда должен быть последним аргументом
   функции и он обязателен при определении общего случая рекурсивной функции. Рекурсивный параметр можно использовать
   как обычную переменную внутри определения. При этом важно понимать, что при вызове `add(2, 5)` значение `y` будет на
   1 меньше, то есть 4.
5. Последняя деталь - в определении общего случая используется переменная `add`. При определении общего случая
   рекурсивной функции можно использовать переменную, названную также, как и функция. Её значение - это значение функции
   с рекурсивным параметром на 1 меньше. Например, если мы вызываем `add(2, 5)`, значением `add` будет
   значение `add(2, 4)` (в объявлении рекурсивной функции аргумент не может называться также, как и функция, ведь
   возникнет путаница).

### Вычисление значения

Мы научились объявлять и определять всякие виды функций, но как посчитать их значение, как что-нибудь выполнить?

Для этого существует синтаксис вызова функции. Например где-нибудь в программе можно написать:

```
add(4, 5);
```

Тогда программа при выполнении вычислит и выведет значение написанного выражения. Все функции в нём должны быть
определены. Переменные внутри него использовать нельзя - только константы.
В программе должен быть ровно 1 (один) вызов функции!

## Функция argmin

Помимо функции `successor` есть ещё одна встроенная функция - `argmin`. Она принимает на вход выражение, которое может
содержать внутри себя `*` в качестве переменной. `argmin` вернёт минимальное натуральное значение `*`, при котором
внутреннее выражение равно 0.

Например, пусть у нас есть функция `is_prime(x)`, которая проверяет число на простоту и возвращает 0, если число
простое, 1 - иначе.
Попробуем написать программу, которая найдёт минимальное простое число, не меньшее 100.

```
min_prime_not_smaller_than(x) = add(
    argmin(is_prime(add(x, *))),
    x
);

min_prime_not_smaller_than(100);
```

Как это работает? `argmin` начнёт перебирать возможные значения `*` - 0, 1, 2, ....
При этом он будет прибавлять к ним значение `x` - в нашем случае это 100.
Получим 100, 101, 102...
Когда он встретит простое число, `is_prime` вернёт 0. Тогда `argmin` вернёт нам значение `*`, при котором это произошло.
Затем нам снова надо прибавить к нему 100 и мы победили!

## include

Большую программу не хочется держать в одном файле, поэтому существует специальная команда `#include`. Это команда не
для интерпретатора, а для препроцессора (на это указывает решётка в начале). Например, вы можете написать в своей
программе `#include "arithmetics"`.
Умная программа подставит на этом месте кусок кода, названный `arithmetics` (как задавать эти названия - позже).
При этом допустимы ромбовидные инклуды. В этом случае исходный код будет включён в итоговый файл единожды в месте самого
раннего инклуда кода с таким именем.

## Запуск программ

Будем считать, что у нас уже есть некоторая программа, которая вычисляет ответ на главный вопрос математической логики.
Как её запустить?

Для этого и нужен этот проект. Необходимо сохранить программу в каком-нибудь файле. Пусть в нашем случае -
это `thequestion.rec`.

Пусть наша программа содержит следующий код:

```
#include "deep_thought"

get_answer() = think_deeply();

get_answer();
```

Чтобы запустить эту программу, нужно ответить на 2 главных вопроса - каков главный файл в вашей программе и от каких
файлов она зависит.
В нашем случае главный файл - это `thequestion.rec`. Он зависит от файла, который содержит код `deep_thought.rec`. Пусть
этот файл называется `deep_thought.rec`.

Тогда, чтобы запустить эту программу, вы должны написать:

```shell
recursive thequestion.rec --include deep_thought:deep_thought.rec
```

Сначала мы указали главный файл, а после `--include` все зависимости. Зависимость можно указать тремя разными способами:

1. Именованный файл: вы пишете `<alias>:<filepath>`. В таком случае, код файла `<filepath>` будет доступен с
   помощью `#include <alias>`.
2. Безымянный файл: вы пишете `<filename>`. В таком случае, код этого файла будет доступен с
   помощью `#include <filename without extension>`
3. Папка: вы пишите `<dirname>`. В таком случае, все файлы из этой папки будут рекурсивно добавлены в качестве
   зависимости. Они будут доступны также, как и в пункте 2, по имени файла без расширения.

Перед каждой зависимостью надо писать `--include` отдельно.

Помимо основных параметров запуска, есть также параметр `-v`. Он указывает на то, насколько подробно интерпретатор будет
выводить все свои действия.

- без `-v`: выводится только результат.
- `-v`: выводятся только предупреждения - некритические ошибки и замечания.
- `-vv`: выводятся предупреждения и основная информация об этапах интерпретации кода.
- `-vvv`: выводится вся информация с подробными деталями о внутренних процессах интерпретатора (**много букав!!!*).

## Примеры

В этом репозитории есть примеры программ. Примеры лежат в папке `examples`, все программы имеют расширение `.rec`.

- Файл `arithmetics.rec` содержит основные арифметические функции. (этот файл предназначен для использования другими
  программами, он не содержит вызова функция, а следовательно не может быть использован в качестве основного файла)
- Файл `is_prime.rec` содержит реализацию метода проверки на простоту. (аналогично пункту 1)
- Файл `test.rec` просто вызывает функцию `is_prime`.
- Файл `secret.rec` запрещено вызывать, ведь он представляет из себя секретное простое решение Великой теоремы Ферма,
  которую лишь какая-то когорта учёных пытается выставить, как трудную проблему. (sarcasm)

## Грамматика (пока что устарела)

```
program := statement ';' program
         | EMPTY
statement := FUNCTION_NAME '(' arguments_list ')' '=' function_value
           | function_call
           
function_value := FUNCTION_NAME '(' composition_arguments ')'
                | CONSTANT
                | VARIABLE_NAME
 
function_call := 

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

## Этапы выполнения программы

1. Препроцессинг: на этом этапе убираются все комментарии, вместо #include подставляются необходимые части других
   файлов.
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