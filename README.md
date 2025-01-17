# Компилятор TeaLang

## Задачи на первую версию
- Модули
- Функции
- Примитивные типы (int, bool, char, указатели)
- Управляющие конструкции (for, if, while, continue, break)
- Работа с памятью (sizeof, alignof)
- external функции

## Инструкции IR
### Essentials
- call
- add
- sub
- mul
- move
- phi (only for SSA)
- return
- branch
- load (load from given address)
- store (store in given address)
- allocate (allocate stack space for variable)

### For optimizations
- select