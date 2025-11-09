# Valgrind Memory Leak Testing Guide

## Обзор

Valgrind - это инструментальный фреймворк для построения инструментов динамического анализа. Мы используем его для обнаружения утечек памяти и других проблем с памятью в FileServer.

## Быстрый старт

### 1. Установка Valgrind

```bash
# Автоматическая установка
make install-valgrind

# Или вручную
sudo apt-get update
sudo apt-get install valgrind
```

### 2. Запуск тестов

```bash
# Запустить все тесты с Valgrind
make valgrind

# Запустить полную проверку основного бинарника
make valgrind-full

# Запустить обычные тесты (без Valgrind)
make test

# Alias для valgrind
make memcheck
```

## Как это работает

### Файлы конфигурации

#### 1. `valgrind.supp` - Suppression File

Содержит известные false positives от сторонних библиотек:
- PostgreSQL (libpq)
- OpenSSL
- C++ standard library
- spdlog
- jwt-cpp

Пример suppression:
```
{
   libpq_init
   Memcheck:Leak
   ...
   fun:PQconnectdb
}
```

#### 2. `scripts/valgrind_test.sh` - Тестовый скрипт

Автоматизирует процесс:
1. Собирает проект с debug symbols (`-g -O0 -fno-omit-frame-pointer`)
2. Запускает каждый тест через Valgrind
3. Анализирует результаты
4. Генерирует отчеты

## Опции Valgrind

### Используемые опции

```bash
--leak-check=full           # Детальная информация об утечках
--show-leak-kinds=all       # Показать все типы утечек
--track-origins=yes         # Отслеживать источники неинициализированных значений
--verbose                   # Подробный вывод
--suppressions=valgrind.supp # Файл подавления ложных срабатываний
--gen-suppressions=all      # Генерировать новые suppressions
--num-callers=30            # Глубина стека вызовов
```

### Типы утечек

#### Definitely Lost (Определенно потеряно)
```
==12345== 100 bytes in 1 blocks are definitely lost
```
**Критично!** Память выделена, но нет указателей на нее.

**Исправление**:
```cpp
// Плохо
char* ptr = new char[100];
// ... забыли delete[]

// Хорошо
std::unique_ptr<char[]> ptr(new char[100]);
// или
std::vector<char> vec(100);
```

#### Indirectly Lost (Косвенно потеряно)
```
==12345== 50 bytes in 2 blocks are indirectly lost
```
Память достижима только через definitely lost блоки.

#### Possibly Lost (Возможно потеряно)
```
==12345== 200 bytes in 5 blocks are possibly lost
```
Valgrind не уверен - есть ли указатели на эту память.

**Часто бывает**:
- Внутренние указатели в середину блока
- Оптимизации компилятора

#### Still Reachable (Всё ещё достижимо)
```
==12345== 1,024 bytes in 10 blocks are still reachable
```
Память не освобождена, но есть указатели при выходе.

**Обычно OK** для:
- Глобальных объектов
- Синглтонов
- Библиотечной инициализации

#### Suppressed (Подавлено)
Известные false positives из suppression файла.

## Интерпретация результатов

### Пример хорошего результата

```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap usage: 1,234 allocs, 1,234 frees, 123,456 bytes allocated
==12345== 
==12345== All heap blocks were freed -- no leaks are possible
==12345== 
==12345== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

✅ **Отлично!** Нет утечек.

### Пример с утечками

```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 100 bytes in 1 blocks
==12345==   total heap usage: 1,235 allocs, 1,234 frees, 123,556 bytes allocated
==12345== 
==12345== 100 bytes in 1 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x4C2E0EF: operator new[](unsigned long) (vg_replace_malloc.c:433)
==12345==    by 0x400A42: main (test.cpp:10)
==12345== 
==12345== LEAK SUMMARY:
==12345==    definitely lost: 100 bytes in 1 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
==12345==         suppressed: 0 bytes in 0 blocks
==12345== 
==12345== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
```

❌ **Проблема!** Утечка 100 байт в test.cpp:10

## Исправление типичных утечек

### 1. Забыли delete/delete[]

```cpp
// Плохо
void bad_function() {
    char* buffer = new char[1024];
    // ... использование
    // забыли delete[]
}

// Хорошо
void good_function() {
    std::vector<char> buffer(1024);
    // автоматическая очистка
}
```

### 2. Не очищаем PGresult в PostgreSQL

```cpp
// Плохо
PGresult* result = PQexec(conn, "SELECT * FROM users");
// забыли PQclear(result)

// Хорошо
PGresult* result = PQexec(conn, "SELECT * FROM users");
if (result) {
    // ... обработка
    PQclear(result);  // Обязательно!
}
```

### 3. Исключения до delete

```cpp
// Плохо
void risky_function() {
    char* data = new char[100];
    may_throw();  // Если выбросит исключение, data потеряется
    delete[] data;
}

// Хорошо
void safe_function() {
    std::unique_ptr<char[]> data(new char[100]);
    may_throw();  // RAII очистит даже при исключении
}
```

### 4. Циклические ссылки в shared_ptr

```cpp
// Плохо
struct Node {
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;  // Циклическая ссылка!
};

// Хорошо
struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;  // Слабая ссылка разрывает цикл
};
```

## Добавление новых suppressions

Если Valgrind сообщает о ложном срабатывании:

1. **Запустить с генерацией suppressions**:
```bash
valgrind --gen-suppressions=all ./test_program
```

2. **Скопировать вывод** в `valgrind.supp`:
```
{
   <название_suppression>
   Memcheck:Leak
   fun:malloc
   fun:проблемная_функция
}
```

3. **Документировать причину** в комментарии:
```
# Known issue in libfoo v1.2.3
# False positive in initialization
{
   libfoo_init
   Memcheck:Leak
   ...
}
```

## CI/CD Integration

### GitHub Actions

```yaml
name: Memory Check

on: [push, pull_request]

jobs:
  valgrind:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y valgrind libpq-dev
      
      - name: Run Valgrind tests
        run: make valgrind
      
      - name: Upload results
        uses: actions/upload-artifact@v2
        with:
          name: valgrind-logs
          path: build/valgrind_*.log
```

## Best Practices

### ✅ DO

1. **Используйте RAII** для управления ресурсами
2. **Используйте smart pointers** вместо raw pointers
3. **Очищайте все ресурсы** (файлы, сокеты, БД соединения)
4. **Тестируйте регулярно** - не ждите release
5. **Фиксируйте утечки сразу** - не накапливайте technical debt

### ❌ DON'T

1. **Не игнорируйте "still reachable"** в библиотечном коде - это OK
2. **Не подавляйте реальные утечки** - только false positives
3. **Не полагайтесь только на Valgrind** - используйте AddressSanitizer тоже
4. **Не тестируйте release builds** - только Debug с символами
5. **Не запускайте production с Valgrind** - слишком медленно

## Альтернативные инструменты

### AddressSanitizer (ASan)

Быстрее чем Valgrind, встроен в компилятор:

```bash
# Сборка с ASan
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" ..
make

# Запуск
./test_program
```

### Memory Sanitizer (MSan)

Для обнаружения неинициализированной памяти:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=memory -fno-omit-frame-pointer -g" ..
```

### Leak Sanitizer (LSan)

Только для поиска утечек (быстрее ASan):

```bash
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=leak -fno-omit-frame-pointer -g" ..
```

## Troubleshooting

### Проблема: Valgrind слишком медленный

**Решение**:
- Используйте `--leak-check=summary` вместо `full`
- Запускайте только измененные тесты
- Используйте AddressSanitizer для разработки

### Проблема: Слишком много suppressions

**Решение**:
- Обновите библиотеки до последних версий
- Используйте `--gen-suppressions=all` для автоматизации
- Группируйте похожие suppressions

### Проблема: False positives от C++ STL

**Решение**:
Добавьте в `valgrind.supp`:
```
{
   stdcpp_locale
   Memcheck:Leak
   ...
   fun:*locale*
}
```

## Полезные ссылки

- [Valgrind Quick Start](https://valgrind.org/docs/manual/quick-start.html)
- [Valgrind Manual](https://valgrind.org/docs/manual/manual.html)
- [Common False Positives](https://wiki.wxwidgets.org/Valgrind_Suppression_File_Howto)
- [PostgreSQL Valgrind](https://wiki.postgresql.org/wiki/Valgrind)

---

**Автор**: FileServer Development Team  
**Дата**: 2024-11-09  
**Версия**: 1.0

