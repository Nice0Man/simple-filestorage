# API Reference

## Обзор

FileServer предоставляет RESTful API для управления файлами. Все endpoints требуют аутентификации через JWT-токены, если не указано иное.

## Базовый URL

```
http://localhost:8080/api/v1
```

## Аутентификация

### POST /auth/login
Аутентификация пользователя и получение JWT-токена.

**Request:**
```json
{
  "username": "user@example.com",
  "password": "password123"
}
```

**Response:**
```json
{
  "success": true,
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "expires_at": "2024-01-01T12:00:00Z",
  "user": {
    "username": "user@example.com",
    "role": "user"
  }
}
```

**Status Codes:**
- `200 OK` - Успешная аутентификация
- `401 Unauthorized` - Неверные учетные данные
- `400 Bad Request` - Некорректный запрос

### POST /auth/logout
Выход из системы и аннулирование токена.

**Headers:**
```
Authorization: Bearer <jwt_token>
```

**Response:**
```json
{
  "success": true,
  "message": "Successfully logged out"
}
```

### GET /auth/me
Получение информации о текущем пользователе.

**Headers:**
```
Authorization: Bearer <jwt_token>
```

**Response:**
```json
{
  "username": "user@example.com",
  "role": "user",
  "created_at": "2024-01-01T10:00:00Z",
  "last_login": "2024-01-01T11:30:00Z"
}
```

## Управление файлами

### GET /files
Получение списка файлов в директории.

**Headers:**
```
Authorization: Bearer <jwt_token>
```

**Query Parameters:**
- `path` (optional) - Путь к директории (по умолчанию: корневая)
- `page` (optional) - Номер страницы (по умолчанию: 1)
- `limit` (optional) - Количество элементов на странице (по умолчанию: 50)
- `sort` (optional) - Сортировка: `name`, `size`, `date` (по умолчанию: `name`)
- `order` (optional) - Порядок: `asc`, `desc` (по умолчанию: `asc`)

**Example:**
```
GET /files?path=/documents&page=1&limit=20&sort=date&order=desc
```

**Response:**
```json
{
  "success": true,
  "data": {
    "files": [
      {
        "name": "document.pdf",
        "path": "/documents/document.pdf",
        "size": 1024000,
        "mime_type": "application/pdf",
        "is_directory": false,
        "created_at": "2024-01-01T10:00:00Z",
        "modified_at": "2024-01-01T11:00:00Z"
      }
    ],
    "pagination": {
      "page": 1,
      "limit": 20,
      "total": 1,
      "total_pages": 1
    }
  }
}
```

### POST /files/upload
Загрузка файла на сервер.

**Headers:**
```
Authorization: Bearer <jwt_token>
Content-Type: multipart/form-data
```

**Form Data:**
- `file` - Файл для загрузки
- `path` (optional) - Путь для сохранения файла
- `overwrite` (optional) - Перезаписать существующий файл (true/false)

**Response:**
```json
{
  "success": true,
  "data": {
    "file": {
      "name": "uploaded_file.pdf",
      "path": "/uploads/uploaded_file.pdf",
      "size": 1024000,
      "mime_type": "application/pdf",
      "uploaded_at": "2024-01-01T12:00:00Z"
    }
  },
  "message": "File uploaded successfully"
}
```

**Status Codes:**
- `201 Created` - Файл успешно загружен
- `400 Bad Request` - Некорректный запрос или файл
- `409 Conflict` - Файл уже существует (если overwrite=false)
- `413 Payload Too Large` - Файл слишком большой
- `415 Unsupported Media Type` - Неподдерживаемый тип файла

### GET /files/download/{path}
Скачивание файла с сервера.

**Headers:**
```
Authorization: Bearer <jwt_token>
```

**Path Parameters:**
- `path` - Путь к файлу

**Example:**
```
GET /files/download/documents/report.pdf
```

**Response:**
- Файл в бинарном формате с соответствующими заголовками
- `Content-Type` - MIME-тип файла
- `Content-Disposition` - attachment; filename="report.pdf"
- `Content-Length` - Размер файла

**Status Codes:**
- `200 OK` - Файл успешно отправлен
- `404 Not Found` - Файл не найден
- `403 Forbidden` - Нет доступа к файлу

### DELETE /files/{path}
Удаление файла или директории.

**Headers:**
```
Authorization: Bearer <jwt_token>
```

**Path Parameters:**
- `path` - Путь к файлу или директории

**Query Parameters:**
- `recursive` (optional) - Рекурсивное удаление директории (true/false)

**Example:**
```
DELETE /files/documents/old_report.pdf
DELETE /files/temp?recursive=true
```

**Response:**
```json
{
  "success": true,
  "message": "File deleted successfully"
}
```

**Status Codes:**
- `200 OK` - Файл успешно удален
- `404 Not Found` - Файл не найден
- `403 Forbidden` - Нет прав на удаление
- `400 Bad Request` - Нельзя удалить непустую директорию без recursive=true

### PUT /files/move
Перемещение или переименование файла.

**Headers:**
```
Authorization: Bearer <jwt_token>
Content-Type: application/json
```

**Request:**
```json
{
  "source": "/documents/old_name.pdf",
  "destination": "/documents/new_name.pdf",
  "overwrite": false
}
```

**Response:**
```json
{
  "success": true,
  "message": "File moved successfully",
  "data": {
    "old_path": "/documents/old_name.pdf",
    "new_path": "/documents/new_name.pdf"
  }
}
```

### POST /files/copy
Копирование файла.

**Headers:**
```
Authorization: Bearer <jwt_token>
Content-Type: application/json
```

**Request:**
```json
{
  "source": "/documents/original.pdf",
  "destination": "/backups/original_copy.pdf",
  "overwrite": false
}
```

**Response:**
```json
{
  "success": true,
  "message": "File copied successfully",
  "data": {
    "source_path": "/documents/original.pdf",
    "destination_path": "/backups/original_copy.pdf"
  }
}
```

## Управление директориями

### POST /directories
Создание новой директории.

**Headers:**
```
Authorization: Bearer <jwt_token>
Content-Type: application/json
```

**Request:**
```json
{
  "path": "/new_directory",
  "recursive": true
}
```

**Response:**
```json
{
  "success": true,
  "message": "Directory created successfully",
  "data": {
    "path": "/new_directory",
    "created_at": "2024-01-01T12:00:00Z"
  }
}
```

## Поиск

### GET /search
Поиск файлов по имени или содержимому.

**Headers:**
```
Authorization: Bearer <jwt_token>
```

**Query Parameters:**
- `query` - Поисковый запрос
- `path` (optional) - Путь для поиска (по умолчанию: корневая)
- `type` (optional) - Тип поиска: `name`, `content`, `both` (по умолчанию: `name`)
- `page` (optional) - Номер страницы
- `limit` (optional) - Количество результатов на странице

**Example:**
```
GET /search?query=report&path=/documents&type=name&page=1&limit=10
```

**Response:**
```json
{
  "success": true,
  "data": {
    "results": [
      {
        "name": "annual_report.pdf",
        "path": "/documents/annual_report.pdf",
        "size": 2048000,
        "mime_type": "application/pdf",
        "relevance_score": 0.95,
        "match_type": "name"
      }
    ],
    "pagination": {
      "page": 1,
      "limit": 10,
      "total": 1,
      "total_pages": 1
    }
  }
}
```

## Системная информация

### GET /health
Проверка состояния сервера (не требует аутентификации).

**Response:**
```json
{
  "status": "healthy",
  "timestamp": "2024-01-01T12:00:00Z",
  "version": "1.0.0",
  "uptime_seconds": 3600,
  "system": {
    "cpu_usage": 15.5,
    "memory_usage": 256000000,
    "disk_usage": {
      "total": 1000000000000,
      "used": 500000000000,
      "free": 500000000000
    }
  }
}
```

### GET /metrics
Метрики Prometheus (не требует аутентификации).

**Response:**
```
# HELP fileserver_requests_total Total number of HTTP requests
# TYPE fileserver_requests_total counter
fileserver_requests_total{method="GET",endpoint="/files"} 1234

# HELP fileserver_request_duration_seconds Request duration in seconds
# TYPE fileserver_request_duration_seconds histogram
fileserver_request_duration_seconds_bucket{le="0.1"} 100
fileserver_request_duration_seconds_bucket{le="0.5"} 200
fileserver_request_duration_seconds_bucket{le="1.0"} 250
fileserver_request_duration_seconds_bucket{le="+Inf"} 300
```

## Коды ошибок

### Общие коды состояния HTTP

- `200 OK` - Запрос выполнен успешно
- `201 Created` - Ресурс создан успешно
- `400 Bad Request` - Некорректный запрос
- `401 Unauthorized` - Требуется аутентификация
- `403 Forbidden` - Доступ запрещен
- `404 Not Found` - Ресурс не найден
- `409 Conflict` - Конфликт (например, файл уже существует)
- `413 Payload Too Large` - Размер запроса превышает лимит
- `415 Unsupported Media Type` - Неподдерживаемый тип медиа
- `429 Too Many Requests` - Превышен лимит запросов
- `500 Internal Server Error` - Внутренняя ошибка сервера

### Формат ошибок

```json
{
  "success": false,
  "error": {
    "code": "FILE_NOT_FOUND",
    "message": "The requested file was not found",
    "details": {
      "path": "/nonexistent/file.pdf"
    }
  },
  "timestamp": "2024-01-01T12:00:00Z"
}
```

## Rate Limiting

API использует rate limiting для предотвращения злоупотреблений:

- **Лимит:** 60 запросов в минуту на пользователя
- **Burst:** До 10 запросов в секунду
- **Headers:** Информация о лимитах в заголовках ответа

```
X-RateLimit-Limit: 60
X-RateLimit-Remaining: 45
X-RateLimit-Reset: 1640995200
```

## CORS

Сервер поддерживает CORS для веб-приложений:

```
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS
Access-Control-Allow-Headers: Content-Type, Authorization
```
