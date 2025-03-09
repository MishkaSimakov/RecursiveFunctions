# Компилятор

## Введение

Я делаю компилятор для C-подобного языка. Грамматика взята из Cpp2: https://github.com/hsutter/cppfront
На данной итерации на большинство папок в проекте можно не обращать внимание. Основные вещи расположены в:
`tests`, `src/ast`, `src/cli`, `src/interpretation`, `src/lexis`, `src/sources`, `src/syntax`, `src/utils`

## Лексер

Файлы лексера находятся в папке `src/lexis`. Перед использованием его необходимо скомпилировать. В
файле `compile_lexis_table.cpp` находится описание регулярных выражений для каждого токена. По ним строится единый
детерминированный конечный автомат (с небольшими дополнениями), согласно которому будет работать лексер. Автомат
сохраняется в файле `lexis.lx`.

Сам лексер находится в файле `LexicalAnalyzer.h`. Он работает подобно потоку, не сохраняя все обработанные токены.
У него есть два основных метода:

- `current_token` возвращает токен, на котором сейчас находится указатель лексера
- `next_token` смещает указатель лексера на (1 токен)* и возвращает новый текущий токен

*: есть технические виды токенов (например комментарий), которые лексер пропускает всегда

Виды токенов описаны в файле `Token.h`. Там используется макрос для создания "умного" enum. Он ведёт себя так же, как и
обычный enum почти во всех случаях, но предоставляет удобные методы, такие как `.to_string()` и т.д. Токен помимо своего
типа хранит диапазон символов, который он покрывает.

Чтобы не копировать повсюду строки, есть класс `SourceManager`. Он единожды загружает текст программы в память, а после
предоставляет способ получения "view" на этот текст программы. Для описания позиций в этом тексте программы существуют
классы `SourceLocation` и `SourceRange`.

## Парсер

Для парсинга используется LR(1)-парсер. Файлы парсера находятся в `src/syntax`. Перед использованием его необходимо
скомпилировать. Он берёт грамматику, описанную в файле `grammar.txt`, составляет LR(1)-таблицу и сохраняет её в
файле `grammar.lr`. Также автоматически генерируется файл `BuildersRegistry.h`. Он необходим, чтобы парсер при
сворачивании правила вызывал правильную функцию для построения AST-дерева.

LR-парсер описан в файле `lr/LRParser.h`. Помимо непосредственно построения AST парсер обрабатывает ошибки. Для этого
есть класс `RecoveryTree` в файле `LRParser.cpp`, который отвечает за механизм восстановления после ошибок. В будущем я
планирую разделить две эти структуры полностью, чтобы `RecoveryTree` было независимой от парсера структурой.

## Интерпретация

Для исполнения кода создан специальный визитор - `InterpreterASTVisitor`. Он находится в папке `src/interpretation`.
Сейчас он устроен очень просто: он проходит AST дерево и вычисляет значение выражений, начиная с листьев. Переменные
сохраняются в мапе `variables_`, значения выражений в `values_`.

## Сборка и запуск

Перед запуском CLI или тестов необходимо сделать следующее:
1. Подготавливаем папку для сборки:
    ```shell
    mkdir build
    cd build
    cmake ..
    ```
2. Компилируем лексер (при исполнении могут быть конфликты между токенами, так и должно быть):
   ```shell
   cmake --build . --target CompileLexisTable
   ./src/lexis/CompileLexisTable
   ```
3. Компилируем парсер
   ```shell
   cmake --build . --target CompileGrammar
   ./src/syntax/CompileGrammar
   ```

Для удобной работы с компилятором сделана сборка Cli:
```shell
cmake --build . --target Cli
```

Теперь можно запустить компилятор (который пока что интерпретатор), передав ему в качестве аргумента файлы, которые нужно скомпилировать (пока что нужно передать ровно 1 файл с функцией main, как описано в задании 1-ой итерации).
```shell
./tlang {filepath}
```

Можно добавить атрибут `--dump-ast`, чтобы вывести AST.

В папке `examples` есть пример простой программы, которую можно запустить. Также в папке `tests/unit/interpretation` есть тесты для интерпретатора, где исполняется ещё несколько программ.

## Тестирование
В папке `tests` находятся базовые тесты для лексера, парсера и интерпретатора. Чтобы их запустить выполните шаги 1-3 из предыдущего раздела, а после выполните:
```shell
cmake --build . --target Tests
./tests/Tests
```