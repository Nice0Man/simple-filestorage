# ✅ Проверка установки CI/CD

## Быстрая проверка

### 1. Проверка YAML синтаксиса
```bash
python3 -c "import yaml; [yaml.safe_load(open(f)) for f in ['.github/workflows/ci.yml', '.github/workflows/pr-check.yml', '.github/workflows/release.yml', '.pre-commit-config.yaml']]" && echo "✅ Все YAML файлы валидны"
```

### 2. Структура файлов
```bash
tree .github/ -L 2
```

### 3. Локальный тест
```bash
bash scripts/test-ci-locally.sh
```

## Что должно работать

✅ GitHub Workflows (3 файла)
✅ Pre-commit конфигурация
✅ Clang-format конфигурация
✅ Скрипты установки и тестирования
✅ Документация (4+ файлов)
✅ Шаблоны PR и commit конвенции

## Следующие шаги

1. **Установить pre-commit:**
   ```bash
   bash scripts/setup-precommit.sh
   ```

2. **Протестировать локально:**
   ```bash
   bash scripts/test-ci-locally.sh
   ```

3. **Закоммитить изменения:**
   ```bash
   git add .
   git commit -m "feat: add CI/CD infrastructure"
   git push origin main
   ```

4. **Проверить на GitHub:**
   - Открыть Actions tab
   - Убедиться что workflows запустились

## Файлы созданы

### GitHub Actions
- `.github/workflows/ci.yml` - Основной CI
- `.github/workflows/pr-check.yml` - Проверка PR
- `.github/workflows/release.yml` - Релизы

### Конфигурация
- `.pre-commit-config.yaml` - Pre-commit hooks
- `.clang-format` - Стиль кода
- `.secrets.baseline` - Baseline секретов
- `.github/labeler.yml` - Авто-лейблы

### Документация
- `docs/development.md` - Гайд разработчика
- `docs/ci-cd.md` - CI/CD документация
- `docs/test-workflows.md` - Тестирование
- `docs/quick-start-ci.md` - Быстрый старт
- `.github/COMMIT_CONVENTION.md` - Commit конвенции
- `.github/PULL_REQUEST_TEMPLATE.md` - PR шаблон
- `CI-CD-SUMMARY.md` - Краткая сводка

### Скрипты
- `scripts/setup-precommit.sh` - Установка (Linux)
- `scripts/setup-precommit.ps1` - Установка (Windows)
- `scripts/test-ci-locally.sh` - Тест (Linux)
- `scripts/test-ci-locally.ps1` - Тест (Windows)
