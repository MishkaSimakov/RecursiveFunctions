# Интерпретатор для рекурсивных функций Гёделя

Набор инструментов для исполнения и отладки рекурсивных функций Гёделя.

## Содержание

- [Кто такие эти функции?](#functions)
- [Язык для описания программ](#lang)
- [Запуск программ](#execution)
- [Примеры](#examples)
- [Детали работы интерпретатора](#details)
- [Формальное описание грамматики](#grammar)
- [Байт-код](https://github.com/MishkaSimakov/RecursiveFunctions/blob/e97ce05f71ea062df9c577d3468f94674ffa6adc/src/compilation/README.md)
- [Отладчик](#debugger)
- [Сборка и тестирование](#buildntest)
- [Идеи на будущее](#future)

<a name="functions">
<h2>
Кто такие эти функции?
</h2>
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

<a name="lang">
<h2>
Язык для описания программ
</h2>
</a>

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

### Функция argmin

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

### include

Большую программу не хочется держать в одном файле, поэтому существует специальная команда `#include`. Это команда не
для интерпретатора, а для препроцессора (на это указывает решётка в начале). Например, вы можете написать в своей
программе `#include "arithmetics"`.
Умная программа подставит на этом месте кусок кода, названный `arithmetics` (как задавать эти названия - позже).
При этом допустимы ромбовидные инклуды. В этом случае исходный код будет включён в итоговый файл единожды в месте самого
раннего инклуда кода с таким именем.

<a name="execution">
<h2>
Запуск программ
</h2>
</a>

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
recurator thequestion.rec --include deep_thought:deep_thought.rec
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
- `-vvv`: выводится вся информация с подробными деталями о внутренних процессах интерпретатора (**много букав!!!**).

<a name="examples">
<h2>
Примеры
</h2>
</a>

В этом репозитории есть примеры программ. Примеры лежат в папке `examples`, все программы имеют расширение `.rec`.

- Файл `arithmetics.rec` содержит основные арифметические функции. (этот файл предназначен для использования другими
  программами, он не содержит вызова функция, а следовательно не может быть использован в качестве основного файла)
- Файл `is_prime.rec` содержит реализацию метода проверки на простоту. (аналогично пункту 1)
- Файл `test.rec` просто вызывает функцию `is_prime`.
- Файл `secret.rec` запрещено вызывать, ведь он представляет из себя секретное простое решение Великой теоремы Ферма,
  которую лишь какая-то когорта учёных пытается выставить, как трудную проблему. (sarcasm)

<a name="details">
<h2>
Детали интерпретатора
</h2>
</a>

Между получением файла на вход и его исполнением происходит несколько больших шагов. Каждый из них преобразует код так,
чтобы с ним было удобнее работать на следующих этапах. При этом на каждом из этапов отлавливается множество
потенциальных ошибок.
Итак, после вызова интерпретатора ваш код проходит следующие этапы:

1. Препроцессинг: препроцессор принимает на вход главный файл и его зависимости. Препроцессор подставляет код из файлов
   вместо `#include`, удаляет комментарии и лишние символы (например, большинство пробелов). На выходе препроцессор
   выдаёт одну большую строку из кода.
2. Лексический анализ: на этом этапе весь код, полученный после препроцессинга, делится на токены. Токен - это
   минимальная структурная единица в коде для интерпретатора - атомы, из которых затем составляются молекулы и целые
   объекты. Почти всегда один токен - это один символ. Однако название переменной или константа - это один токен, хотя
   состоят они из множества символов. Например код `function(a) = 123;` будет разбит на следующие
   токены: `function`, `(`, `a`, `)`, `=`, `123`, `;`. Также каждый токен для удобства сразу имеет свой тип - левая или
   правая скобка, запятая, точка с запятой, название (identifier) и т.д.
3. Синтаксический анализ: на этом этапе массив их токенов превращается в абстрактное синтаксическое дерево. Такая
   структура данных гораздо лучше описывает вложенную структуру кода. Например, если в вашем коде встречается
   вызов `f(g(x), t(y))`, то в дереве это будет представлено вершиной `f`, дети которой - это функции `g` и `t`.
   На этом этапе отлавливаются все ошибки в синтаксисе - лишняя скобка, отсутствующая запятая и т.д. - всё это
   отлавливается здесь.
4. Построение дерева для компиляции: абстрактное синтаксическое дерево превращается в другое дерево, которое содержит
   больше информации для компилятора. Здесь каждой переменной и функции выдаётся свой уникальный индекс, проверяется,
   что во всех вызовах функций правильное количество аргументов и т.д.
5. Компиляция в байт-код: несмотря на то, что это интерпретатор, в нём всё равно есть этап компиляции. Только не в
   assembler, а в байт-код. Это специальный язык, который содержит минимальное количество максимально простых функций
   для специальной виртуальной машины. Он предназначен для максимально быстрого исполнения. При этом компиляция
   происходит рекурсивно. Мы последовательно посещаем вершины нашего дерева, компилируя сначала детей, а после -
   родителей. На выходе мы получаем длинную последовательность команд.
6. Исполнение байт-кода: тут всё просто, полученный байт-код выполняется, пока не будет получен результат. Однако и
   исполнять его можно по-разному. Есть два основных варианта:
    - Максимально быстрое исполнение без лишних действий. На выходе мы получаем лишь результат работы программы.
    - Более медленное исполнение, при котором собирается вся информация. В процессе выполнения мы можем посмотреть
      промежуточные результаты, проверить, не выходим ли мы за границы выделенной памяти.

<a name="grammar">
<h2>
Грамматика
</h2>
</a>

Для описания грамматики языка используется контекстно-свободная грамматика.
Для проверки того, удовлетворяет ли код программы этой грамматике используется хитрый алгоритм, который в худшем случае
за экспоненциальное время выполняет разбор. Код этого чуда можно найти в папке `src/syntax/buffalo`.

Терминальные символы написаны большими буквами или символами в кавычках. FUNCTION_NAME, VARIABLE - это один токен типа
Identifier (последовательность строчных и заглавных букв английского алфавита + нижнее подчёркивание), CONSTANT - один
токен типа Constant (последовательность цифр).

```
program := statement ';' program
         | EMPTY
         
statement := FUNCTION_NAME '(' arguments_list ')' '=' function_value
           | function_call
           
function_value := FUNCTION_NAME '(' composition_arguments ')'
                | CONSTANT
                | VARIABLE_NAME
 
function_call := FUNCTION_NAME '(' call_arguments ')'

arguments_list := nonempty_arguments_list | EMPTY
nonempty_arguments_list := VARIABLE_NAME ',' nonempty_arguments_list
                         | recursion_parameter
                         | VARIABLE_NAME
                         
recursion_parameter := VARIABLE_NAME '+' '1'
                     | '0'
                
composition_arguments = nonempty_composition_arguments | EMPTY
nonempty_composition_arguments := FUNCTION_NAME '(' composition_arguments ')' ',' nonempty_composition_arguments
                                | VARIABLE_NAME ',' nonempty_composition_arguments
                                | CONSTANT ',' nonempty_composition_arguments
                                | '*' ',' nonempty_composition_arguments
                                | FUNCTION_NAME '(' composition_arguments ')'
                                | VARIABLE_NAME
                                | CONSTANT
                                | '*'
                                
call_arguments := nonempty_call_arguments | EMPTY
nonempty_call_arguments := FUNCTION_NAME '(' call_arguments ')' ',' nonempty_call_arguments
                         | CONSTANT ',' nonempty_call_arguments
                         | '*' ',' nonempty_call_arguments
                         | FUNCTION_NAME '(' call_arguments ')'
                         | CONSTANT
                         | '*'                  
```

<a name="debugger">
<h2>
Отладчик
</h2>
</a>

**ВАЖНО!** Для изучения этого раздела сначала важно прочитать про байт-код и порядок исполнения программы в целом. Про
это можно
прочесть [здесь](https://github.com/MishkaSimakov/RecursiveFunctions/blob/e97ce05f71ea062df9c577d3468f94674ffa6adc/src/compilation/README.md)

Чтобы подробнее изучить этап исполнения байт-кода можно воспользоваться отладчиком. Для его запуска необходимо передать
флаг `--debug` при запуске интерпретатора.
Все этапы до этапа исполнения байт-кода работают без изменений. Однако перед исполнением байт-кода интерпретатор
переходит в интерактивный режим: он будет ждать от вас команд через stdin. Его действиями можно управлять с
помощью следующих команд:

- `line <index>` - выводит строчку байт-кода с номером `<index>` и несколько строчек рядом с ней.
- `print <call|args|calc>[<from>:<to>]` - выводит значения с соответствующего стека: call - стек вызовов функций, args -
  стек аргументов, calc - стек вычислений. Если значения `<from>` и `<to>` не переданы, то выведет несколько значений
  рядом с текущим положением указателя на этом стеке. Если же переданы, то выведет значения с индексами от `<from>`
  до `<to>`. Например `print call` - выведет несколько значений рядом с верхним элементом на стеке вызовов функций,
  а `print calc[100:200]` выведет значения с 100-го по 200-е со стека вычислений.
- `step` - сделает один шаг исполнения байт-кода, то есть выполнит текущую команду и переместится на следующую позицию.
- `breakpoint <index>` - добавит точку остановки на строчке с номером `<index>`.
- `run` - продолжит исполнение кода, начиная с текущей позиции. Код будет выполняться до тех пор, пока он не завершиться
  или пока исполнение не дойдёт до строчки, на которой установлена точка остановки (при этом эта строчка выполнена не
  будет).

<a name="buildntest">
<h2>
Сборка и тестирование
</h2>
</a>

Проект использует библиотеки [argparse](https://github.com/p-ranav/argparse), [fmt](https://github.com/fmtlib/fmt)
и [google test](https://github.com/google/googletest). Используется стандарт C++23.

Для сборки проекта необходимо выполнить следующие шаги, начиная из корня проекта:

```shell
git submodule update --init
mkdir build
cd build
cmake ..
cmake --build .
cmake --install .
```

После этого вам станет доступна команда `recurator`.

Также для быстрой настройки окружения можно использовать Docker container.
Dockerfile для сборки образа находится в корне проекта. Чтобы собрать образ и запустить его
достаточно выполнить следующие шаги из корня проекта:

```shell
docker build . --tag=recurator-dev
docker run -it --rm --mount type=bind,source=$(pwd),destination=/recurator recurator-dev
```

<a name="future">
<h2>
Идеи на будущее
</h2>
</a>

- [ ] Реализовать хороший парсер грамматики (какие-то страшные viable prefixes и non deterministic finite automata).
- [ ] Сделать класс, хранящий перевод программы с любого этапа интерпретации в исходный (это нужно для красивых и
  понятных ошибок, ведь уже после действия препроцессора код сложно узнать)
- [ ] Реализовать крутые ошибки у парсера для грамматики (то есть выводить именно место, в котором находится ошибка)
- [ ] Сделать отладчик не для байт-кода, а для исходной программы
- [ ] Добавить массивы в язык (всем известно, что массивы можно реализовать и без дополнений к языку, но программы с
  ними не успеют выполниться к концу вселенной, поэтому хочется встроить массивы в язык и сделать базовые функции для
  работы с ними встроенными и нерекурсивными)
- [ ] Компиляция в ассемблер (это позволит сильно ускорить исполнение кода, но нужно изучить ассемблер, а у меня сессия
  на носу)
- [ ] Оптимизации байт-кода (самая первая и основная идея - подстановка тела функции вместо их вызова)
