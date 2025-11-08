# Проверка CI Статуса

## 📊 Статус ветки dev

### ✅ Текущая информация

- **Ветка:** dev
- **Последний коммит:** cb735604 - "Update ci.yml"
- **Автор:** Eugene
- **Статус синхронизации:** ✓ Синхронизирована с remote
- **Workflows:** 3 активных workflow файла

### 🔍 Быстрая проверка

```bash
# Проверить статус любой ветки
bash scripts/check-ci-status.sh dev

# Или для другой ветки
bash scripts/check-ci-status.sh master
```

## 🌐 GitHub Actions (Online)

### Прямые ссылки

1. **Все Actions запуски:**
   ```
   https://github.com/Nice0Man/simple-filestorage/actions
   ```

2. **Коммиты в ветке dev:**
   ```
   https://github.com/Nice0Man/simple-filestorage/commits/dev
   ```

3. **Сравнение веток (dev vs master):**
   ```
   https://github.com/Nice0Man/simple-filestorage/compare/master...dev
   ```

### Как проверить на GitHub

1. Откройте репозиторий на GitHub
2. Перейдите на вкладку **Actions**
3. В левой панели выберите workflow (CI, PR Check, Release)
4. Отфильтруйте по ветке **dev** в верхнем фильтре
5. Посмотрите статус последних запусков:
   - ✅ Зелёная галочка = успешно
   - ❌ Красный крестик = ошибка
   - 🟡 Жёлтый кружок = в процессе
   - ⚪ Серый кружок = ожидание

## 💻 Локальная проверка

### Использование скрипта

```bash
# Базовая проверка
bash scripts/check-ci-status.sh dev

# Что проверяет скрипт:
# ✓ Информация о ветке
# ✓ Статус синхронизации с remote
# ✓ Наличие workflow файлов
# ✓ Валидность YAML
# ✓ Ссылки для проверки на GitHub
```

### Ручная проверка

```bash
# 1. Проверить текущую ветку
git branch --show-current

# 2. Последний коммит
git log -1 --oneline

# 3. Статус синхронизации
git fetch origin dev
git status

# 4. Валидация workflows
python3 -c "import yaml; [yaml.safe_load(open(f)) for f in ['.github/workflows/ci.yml', '.github/workflows/pr-check.yml', '.github/workflows/release.yml']]"
```

## 🔧 GitHub CLI (gh)

### Установка

**Linux/macOS:**
```bash
# Homebrew
brew install gh

# Debian/Ubuntu
curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | sudo dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null
sudo apt update
sudo apt install gh
```

**Windows:**
```powershell
# Chocolatey
choco install gh

# Scoop
scoop install gh
```

### Использование

```bash
# Авторизация
gh auth login

# Проверить статус CI для ветки dev
gh run list --branch dev --limit 10

# Детали конкретного запуска
gh run view <run-id>

# Следить за запуском в реальном времени
gh run watch

# Логи неудачного запуска
gh run view <run-id> --log-failed
```

### Полезные команды

```bash
# Список всех workflow
gh workflow list

# Запустить workflow вручную
gh workflow run ci.yml --ref dev

# Статус последнего запуска
gh run list --limit 1 --branch dev

# Проверить статус конкретного workflow
gh run list --workflow=ci.yml --branch dev

# Скачать артефакты
gh run download <run-id>
```

## 📈 CI Статусы

### Типы статусов

| Статус | Значение | Действие |
|--------|----------|----------|
| ✅ Success | Все проверки прошли | Можно мержить |
| ❌ Failure | Есть ошибки | Нужно исправить |
| 🟡 In Progress | Выполняется | Подождать |
| ⚪ Queued | В очереди | Подождать |
| ⭕ Skipped | Пропущен | Проверить условия |
| 🚫 Cancelled | Отменён | Перезапустить |

### Что проверяется в CI

#### Workflow: CI (ci.yml)

**Триггеры:** Push в main/develop, PR к main/develop

**Проверки:**
1. **Build and Test Matrix**
   - Ubuntu 20.04 + GCC 11, Clang 14
   - Ubuntu 22.04 + GCC 11/12, Clang 14/15
   - Debug и Release сборки

2. **Code Quality**
   - clang-format (форматирование)
   - cppcheck (статический анализ)

3. **Docker Build**
   - Сборка Docker образа
   - Тест запуска

#### Workflow: PR Check (pr-check.yml)

**Триггеры:** Открытие/обновление PR

**Проверки:**
1. Валидация названия PR (conventional commits)
2. Проверка размера PR
3. Обнаружение конфликтов
4. Автоматическая маркировка labels

#### Workflow: Release (release.yml)

**Триггеры:** Push тега v*.*.*

**Действия:**
1. Создание GitHub Release
2. Сборка артефактов
3. Публикация Docker образа

## 🐛 Решение проблем

### CI не запускается

**Проблема:** Workflow не триггерится после push

**Решение:**
```bash
# 1. Проверить что workflows в main ветке
git checkout main
ls -la .github/workflows/

# 2. Убедиться что Actions включены
# GitHub → Settings → Actions → General → Allow all actions

# 3. Проверить триггеры в workflow файле
cat .github/workflows/ci.yml | grep -A 5 "on:"
```

### Как перезапустить CI

**На GitHub:**
1. Actions → выбрать неудачный запуск
2. Кнопка "Re-run jobs" → "Re-run failed jobs"

**С GitHub CLI:**
```bash
# Найти ID запуска
gh run list --branch dev --limit 5

# Перезапустить
gh run rerun <run-id>

# Или перезапустить только failed jobs
gh run rerun <run-id> --failed
```

### CI падает на конкретном этапе

**1. Build failures:**
```bash
# Локально протестировать
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

**2. Test failures:**
```bash
cd build
ctest --verbose --output-on-failure
```

**3. Formatting issues:**
```bash
# Исправить форматирование
find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format -i
git add .
git commit -m "style: apply clang-format"
```

## 📊 Мониторинг

### Badges для README

Добавьте в README.md:

```markdown
![CI Status](https://github.com/Nice0Man/simple-filestorage/actions/workflows/ci.yml/badge.svg?branch=dev)
```

### Webhook уведомления

**Discord/Slack:**
1. Settings → Webhooks → Add webhook
2. Events: Workflow runs
3. Настроить фильтры по ветке

### Email уведомления

GitHub → Settings → Notifications → Actions
- Выбрать когда получать уведомления
- Настроить фильтры

## 🔗 Полезные ссылки

- **GitHub Actions Documentation:** https://docs.github.com/en/actions
- **GitHub CLI Documentation:** https://cli.github.com/manual/
- **Workflow Syntax:** https://docs.github.com/en/actions/reference/workflow-syntax-for-github-actions

## 📝 Примеры использования

### Ежедневная проверка

```bash
# Утром перед работой
bash scripts/check-ci-status.sh dev

# Посмотреть что изменилось
git fetch origin dev
git log HEAD..origin/dev --oneline

# Обновить локальную ветку
git pull origin dev
```

### Перед созданием PR

```bash
# 1. Проверить что все чисто
bash scripts/test-ci-locally.sh

# 2. Запушить
git push origin feature/my-feature

# 3. Проверить CI статус
gh run list --branch feature/my-feature --limit 1

# 4. Если всё OK, создать PR
gh pr create --title "feat: my feature" --body "Description"
```

### После мержа в dev

```bash
# Проверить что CI прошёл
gh run list --branch dev --limit 1

# Если есть проблемы
gh run view --log-failed

# Быстрый фикс
git checkout dev
git pull
# fix issues
git add .
git commit -m "fix: resolve CI issues"
git push
```

---

**Быстрые команды:**

```bash
# Проверить статус
bash scripts/check-ci-status.sh dev

# Открыть Actions на GitHub
xdg-open https://github.com/Nice0Man/simple-filestorage/actions  # Linux
open https://github.com/Nice0Man/simple-filestorage/actions      # macOS
start https://github.com/Nice0Man/simple-filestorage/actions     # Windows
```

