# CI/CD Setup Summary

## ✅ Что добавлено

### 🔄 GitHub Workflows

1. **`.github/workflows/ci.yml`** - Основной CI pipeline
   - Сборка на Ubuntu 20.04 и 22.04
   - Тестирование с GCC 11/12 и Clang 14/15
   - Проверка качества кода (clang-format, cppcheck)
   - Сборка Docker образа

2. **`.github/workflows/pr-check.yml`** - Проверка Pull Request
   - Валидация названия PR (conventional commits)
   - Проверка размера PR
   - Обнаружение конфликтов
   - Автоматическая маркировка labels

3. **`.github/workflows/release.yml`** - Автоматизация релизов
   - Создание GitHub релизов при push тегов
   - Сборка бинарных артефактов
   - Публикация Docker образов в GHCR

### 🪝 Pre-commit Hooks

- **`.pre-commit-config.yaml`** - Конфигурация hooks
  - Проверка форматирования (clang-format)
  - Статический анализ (cppcheck)
  - Валидация YAML/JSON
  - Обнаружение секретов
  - Проверка include guards
  - Обнаружение debug prints

### 📋 Конфигурация

- **`.clang-format`** - Стиль форматирования C++ кода
- **`.secrets.baseline`** - Baseline для detect-secrets
- **`.github/labeler.yml`** - Правила автоматической маркировки PR
- **`.github/PULL_REQUEST_TEMPLATE.md`** - Шаблон PR
- **`.github/COMMIT_CONVENTION.md`** - Конвенция commit сообщений

### 📚 Документация

- **`docs/development.md`** - Полное руководство разработчика
- **`docs/ci-cd.md`** - Документация CI/CD
- **`docs/test-workflows.md`** - Тестирование workflows
- **`docs/quick-start-ci.md`** - Быстрый старт

### 🔧 Скрипты

- **`scripts/setup-precommit.sh`** - Установка pre-commit (Linux/macOS)
- **`scripts/setup-precommit.ps1`** - Установка pre-commit (Windows)
- **`scripts/test-ci-locally.sh`** - Локальное тестирование CI (Linux/macOS)
- **`scripts/test-ci-locally.ps1`** - Локальное тестирование CI (Windows)

### 🔐 Обновлённые файлы

- **`.gitignore`** - Добавлены исключения для pre-commit cache

## 🚀 Быстрый старт

### 1. Установка Pre-commit Hooks

**Linux/macOS:**
```bash
bash scripts/setup-precommit.sh
```

**Windows:**
```powershell
powershell scripts/setup-precommit.ps1
```

### 2. Тестирование локально

**Linux/macOS:**
```bash
bash scripts/test-ci-locally.sh
```

**Windows:**
```powershell
powershell scripts/test-ci-locally.ps1
```

### 3. Первый коммит

```bash
# Отформатировать весь код
pre-commit run --all-files

# Добавить изменения
git add .

# Коммит с правильным форматом
git commit -m "feat: add CI/CD configuration"

# Push для запуска CI
git push origin main
```

### 4. Проверка на GitHub

1. Перейти в **Actions** tab на GitHub
2. Посмотреть запущенные workflows
3. Убедиться что все проверки прошли ✅

## 📖 Документация

### Основные руководства

- **Быстрый старт:** [`docs/quick-start-ci.md`](docs/quick-start-ci.md)
- **Разработка:** [`docs/development.md`](docs/development.md)
- **CI/CD:** [`docs/ci-cd.md`](docs/ci-cd.md)
- **Тестирование:** [`docs/test-workflows.md`](docs/test-workflows.md)

### Конвенции

- **Commit сообщения:** [`.github/COMMIT_CONVENTION.md`](.github/COMMIT_CONVENTION.md)
- **Pull Request:** [`.github/PULL_REQUEST_TEMPLATE.md`](.github/PULL_REQUEST_TEMPLATE.md)

## 🔍 Проверка workflows

### Локальная проверка YAML

```bash
# Проверить синтаксис всех workflows
python3 -c "import yaml; yaml.safe_load(open('.github/workflows/ci.yml'))"
python3 -c "import yaml; yaml.safe_load(open('.github/workflows/pr-check.yml'))"
python3 -c "import yaml; yaml.safe_load(open('.github/workflows/release.yml'))"
```

### Использование Act (опционально)

```bash
# Установить act: https://github.com/nektos/act
# Запустить workflows локально
act -l                    # Список workflows
act push                  # Симулировать push
act pull_request          # Симулировать PR
```

## 📋 Pre-commit Hooks

### Что проверяется автоматически

✅ **При каждом коммите:**
- Удаление trailing whitespace
- Фиксация конца файлов
- Валидация YAML/JSON
- Форматирование кода (clang-format)
- Статический анализ (cppcheck)
- Include guards
- Debug prints
- Секреты в коде

### Ручное использование

```bash
# Запустить все hooks
pre-commit run --all-files

# Запустить конкретный hook
pre-commit run clang-format

# Обновить hooks
pre-commit autoupdate

# Пропустить hooks (не рекомендуется)
git commit --no-verify
```

## 🏗️ CI Pipeline

### Матрица сборки

| OS           | Компиляторы          | Build Types    |
|--------------|----------------------|----------------|
| Ubuntu 20.04 | GCC 11, Clang 14     | Debug, Release |
| Ubuntu 22.04 | GCC 11/12, Clang 14/15 | Debug, Release |

### Что проверяет CI

1. ✅ Компиляция на разных платформах и компиляторах
2. ✅ Все unit тесты проходят
3. ✅ Форматирование кода (clang-format)
4. ✅ Статический анализ (cppcheck)
5. ✅ Сборка Docker образа

## 📦 Создание релиза

### Шаги

```bash
# 1. Подготовить релиз
git add .
git commit -m "chore: prepare release v1.0.0"
git push origin main

# 2. Создать и запушить тег
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0

# 3. Дождаться завершения workflow
# GitHub Actions автоматически:
# - Создаст GitHub Release
# - Соберёт бинарные артефакты
# - Опубликует Docker образ
```

### Формат версий (Semantic Versioning)

- `v1.0.0` - Стабильный релиз
- `v1.1.0` - Новые функции
- `v1.1.1` - Исправления багов
- `v2.0.0` - Breaking changes
- `v1.0.0-alpha` - Alpha версия
- `v1.0.0-beta.1` - Beta версия
- `v1.0.0-rc.1` - Release candidate

## 🔧 Формат Commit сообщений

### Базовый формат

```
type(scope): subject

body (опционально)

footer (опционально)
```

### Типы

- `feat` - Новая функциональность
- `fix` - Исправление бага
- `docs` - Документация
- `style` - Форматирование
- `refactor` - Рефакторинг
- `perf` - Оптимизация производительности
- `test` - Тесты
- `build` - Сборка
- `ci` - CI/CD
- `chore` - Поддержка

### Примеры

```bash
feat(auth): add JWT token validation
fix(file-manager): resolve memory leak in upload handler
docs(readme): update installation instructions
style: apply clang-format to all files
refactor(database): extract connection pool logic
```

## ⚙️ Установка инструментов

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    cmake ninja-build \
    g++-12 clang-14 clang-format-14 \
    cppcheck libssl-dev \
    postgresql-server-dev-all \
    pkg-config python3 python3-pip

pip3 install --user pre-commit
```

### macOS

```bash
brew install cmake ninja gcc llvm cppcheck python3
pip3 install pre-commit
```

### Windows

```powershell
# С Chocolatey
choco install -y cmake ninja llvm python3

pip install pre-commit
```

## 🐛 Решение проблем

### Pre-commit hook не срабатывает

```bash
# Переустановить hooks
pre-commit uninstall
pre-commit install

# Проверить установку
pre-commit --version
```

### CI падает на форматировании

```bash
# Отформатировать весь код
pre-commit run clang-format --all-files

# Или вручную
find src include tests -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | \
    xargs clang-format -i

# Закоммитить изменения
git add .
git commit -m "style: apply clang-format"
```

### Тесты падают в CI

```bash
# Запустить тесты локально
cd build
ctest --verbose --output-on-failure

# Запустить конкретный тест
./bin/fileserver_tests --gtest_filter=TestName
```

### Docker сборка не проходит

```bash
# Собрать без кэша
docker build --no-cache -t fileserver:test .

# Проверить логи
docker build -t fileserver:test . 2>&1 | tee docker-build.log
```

## 📊 Best Practices

### Перед коммитом

1. ✅ Запустить тесты: `cd build && ctest`
2. ✅ Отформатировать код: `pre-commit run --all-files`
3. ✅ Проверить линтер: нет новых предупреждений
4. ✅ Написать хорошее commit сообщение

### Перед Pull Request

1. ✅ Rebase на последний main
2. ✅ Squash связанных коммитов
3. ✅ Обновить документацию
4. ✅ Все CI проверки прошли

### Размер PR

- Стараться < 50 файлов
- Стараться < 1000 строк
- Одна фича/фикс на PR

## 🔗 Полезные ссылки

- **Act** (локальный запуск GitHub Actions): https://github.com/nektos/act
- **ActionLint** (валидация workflows): https://github.com/rhysd/actionlint
- **Conventional Commits**: https://www.conventionalcommits.org/
- **Semantic Versioning**: https://semver.org/
- **Pre-commit**: https://pre-commit.com/
- **GitHub Actions Docs**: https://docs.github.com/en/actions

## 📞 Поддержка

Если возникли вопросы или проблемы:

1. 📖 Проверьте документацию в `docs/`
2. 🔍 Посмотрите примеры в `.github/`
3. 🐛 Создайте issue на GitHub
4. 💬 Спросите в discussions

---

**Готово к использованию! 🎉**

Запустите `bash scripts/setup-precommit.sh` для начала работы.

