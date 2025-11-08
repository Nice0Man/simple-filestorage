# Краткое руководство по запуску FileServer

## Быстрый старт

### 1. Запуск с Docker (рекомендуется)

```bash
# Windows
powershell -ExecutionPolicy Bypass -File build_and_run.ps1

# Linux/macOS
./build_and_run.sh
```

### 2. Проверка работы

После запуска сервер будет доступен по адресу: http://localhost:8080

**Проверка здоровья сервера:**
```bash
curl http://localhost:8080/health
```

**Ожидаемый ответ:**
```json
{
  "status": "healthy",
  "timestamp": 1640995200,
  "version": "1.0.0"
}
```

### 3. Аутентификация

**Получение JWT токена:**
```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'
```

**Ответ:**
```json
{
  "success": true,
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "expires_at": "2024-01-01T12:00:00Z",
  "user": {
    "username": "admin",
    "role": "admin"
  }
}
```

### 4. Основные операции с файлами

**Список файлов:**
```bash
curl -H "Authorization: Bearer <token>" \
  http://localhost:8080/api/v1/files
```

**Загрузка файла:**
```bash
curl -X POST \
  -H "Authorization: Bearer <token>" \
  -F "file=@document.pdf" \
  -F "path=/documents/document.pdf" \
  http://localhost:8080/api/v1/files/upload
```

**Скачивание файла:**
```bash
curl -H "Authorization: Bearer <token>" \
  http://localhost:8080/api/v1/files/download/documents/document.pdf \
  -o document.pdf
```

**Удаление файла:**
```bash
curl -X DELETE \
  -H "Authorization: Bearer <token>" \
  http://localhost:8080/api/v1/files/documents/document.pdf
```

## Учетные записи по умолчанию

| Пользователь | Пароль | Роль |
|-------------|--------|------|
| admin | admin123 | admin |
| user | user123 | user |

## Полезные команды Docker

```bash
# Просмотр логов
docker logs -f fileserver-container

# Остановка сервера
docker stop fileserver-container

# Перезапуск сервера
docker restart fileserver-container

# Удаление контейнера
docker rm fileserver-container

# Просмотр статуса
docker ps
```

## Структура файлов

```
C:\Cpp\FileServer\
├── files/          # Хранилище файлов
├── logs/           # Логи сервера
├── config/         # Конфигурационные файлы
├── docs/           # Документация
└── src/            # Исходный код
```

## Конфигурация

Основные настройки находятся в файле `config/config.json`:

```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080
  },
  "storage": {
    "root_directory": "./files",
    "max_file_size_mb": 100
  },
  "security": {
    "authentication_enabled": true,
    "jwt_secret": "your-secret-key"
  }
}
```

## Troubleshooting

### Проблема: Контейнер не запускается
```bash
# Проверить логи
docker logs fileserver-container

# Проверить порты
netstat -an | findstr 8080
```

### Проблема: Ошибка аутентификации
- Проверьте правильность учетных данных
- Убедитесь, что токен не истек
- Проверьте заголовок Authorization

### Проблема: Файл не загружается
- Проверьте размер файла (лимит 100MB)
- Убедитесь, что тип файла разрешен
- Проверьте права доступа к директории

## API документация

Полная документация API доступна по адресу:
- Swagger UI: http://localhost:8080/docs
- OpenAPI спецификация: `docs/swagger.yaml`

## Поддержка

При возникновении проблем:
1. Проверьте логи: `docker logs fileserver-container`
2. Убедитесь, что Docker запущен
3. Проверьте доступность порта 8080
4. Обратитесь к полной документации в `README.md`
