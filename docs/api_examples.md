# Примеры использования API

## Аутентификация

### Вход в систему

**Запрос:**
```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "admin",
    "password": "admin123"
  }'
```

**Ответ:**
```json
{
  "success": true,
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJmaWxlc2VydmVyIiwidXNlcm5hbWUiOiJhZG1pbiIsImV4cCI6MTY0MTAwODAwMH0.signature",
  "expires_at": "2024-01-01T12:00:00Z",
  "user": {
    "username": "admin",
    "role": "admin",
    "created_at": "2024-01-01T10:00:00Z",
    "last_login": "2024-01-01T11:30:00Z"
  }
}
```

## Управление файлами

### Получение списка файлов

**Запрос:**
```bash
curl -H "Authorization: Bearer <token>" \
  "http://localhost:8080/api/v1/files?path=/documents&page=1&limit=10&sort=name&order=asc"
```

**Ответ:**
```json
{
  "success": true,
  "data": {
    "files": [
      {
        "name": "reports",
        "path": "/documents/reports",
        "size": 0,
        "mime_type": "inode/directory",
        "is_directory": true,
        "created_at": "2024-01-01T10:00:00Z",
        "modified_at": "2024-01-01T10:00:00Z"
      },
      {
        "name": "annual_report.pdf",
        "path": "/documents/annual_report.pdf",
        "size": 2048000,
        "mime_type": "application/pdf",
        "is_directory": false,
        "created_at": "2024-01-01T10:30:00Z",
        "modified_at": "2024-01-01T11:00:00Z"
      }
    ],
    "count": 2,
    "pagination": {
      "page": 1,
      "limit": 10,
      "total": 2,
      "total_pages": 1
    }
  }
}
```

### Загрузка файла

**Запрос:**
```bash
curl -X POST \
  -H "Authorization: Bearer <token>" \
  -F "file=@/path/to/local/document.pdf" \
  -F "path=/documents/uploaded_document.pdf" \
  -F "overwrite=false" \
  http://localhost:8080/api/v1/files/upload
```

**Ответ (успешная загрузка):**
```json
{
  "success": true,
  "message": "File uploaded successfully",
  "data": {
    "file": {
      "name": "uploaded_document.pdf",
      "path": "/documents/uploaded_document.pdf",
      "size": 1024000,
      "mime_type": "application/pdf",
      "is_directory": false,
      "uploaded_at": "2024-01-01T12:00:00Z"
    }
  }
}
```

**Ответ (ошибка - файл существует):**
```json
{
  "success": false,
  "error": "File already exists",
  "code": "FILE_EXISTS",
  "details": {
    "path": "/documents/uploaded_document.pdf"
  },
  "timestamp": "2024-01-01T12:00:00Z"
}
```

### Скачивание файла

**Запрос:**
```bash
curl -H "Authorization: Bearer <token>" \
  http://localhost:8080/api/v1/files/download/documents/annual_report.pdf \
  -o annual_report.pdf
```

**Заголовки ответа:**
```
HTTP/1.1 200 OK
Content-Type: application/pdf
Content-Disposition: attachment; filename="annual_report.pdf"
Content-Length: 2048000
```

### Удаление файла

**Запрос:**
```bash
curl -X DELETE \
  -H "Authorization: Bearer <token>" \
  http://localhost:8080/api/v1/files/documents/old_document.pdf
```

**Ответ:**
```json
{
  "success": true,
  "message": "File deleted successfully"
}
```

### Перемещение/переименование файла

**Запрос:**
```bash
curl -X PUT \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "source": "/documents/old_name.pdf",
    "destination": "/documents/new_name.pdf",
    "overwrite": false
  }' \
  http://localhost:8080/api/v1/files/move
```

**Ответ:**
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

### Копирование файла

**Запрос:**
```bash
curl -X POST \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "source": "/documents/original.pdf",
    "destination": "/backups/original_copy.pdf",
    "overwrite": false
  }' \
  http://localhost:8080/api/v1/files/copy
```

**Ответ:**
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

### Создание директории

**Запрос:**
```bash
curl -X POST \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "path": "/projects/new_project",
    "recursive": true
  }' \
  http://localhost:8080/api/v1/directories
```

**Ответ:**
```json
{
  "success": true,
  "message": "Directory created successfully",
  "data": {
    "path": "/projects/new_project",
    "created_at": "2024-01-01T12:00:00Z"
  }
}
```

## Поиск файлов

### Поиск по имени

**Запрос:**
```bash
curl -H "Authorization: Bearer <token>" \
  "http://localhost:8080/api/v1/search?query=report&path=/documents&type=name&page=1&limit=10"
```

**Ответ:**
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
        "is_directory": false,
        "relevance_score": 0.95,
        "match_type": "name",
        "modified_at": "2024-01-01T11:00:00Z"
      },
      {
        "name": "monthly_report.docx",
        "path": "/documents/reports/monthly_report.docx",
        "size": 512000,
        "mime_type": "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
        "is_directory": false,
        "relevance_score": 0.87,
        "match_type": "name",
        "modified_at": "2024-01-01T09:30:00Z"
      }
    ],
    "pagination": {
      "page": 1,
      "limit": 10,
      "total": 2,
      "total_pages": 1
    }
  }
}
```

## Системная информация

### Проверка здоровья

**Запрос:**
```bash
curl http://localhost:8080/health
```

**Ответ:**
```json
{
  "status": "healthy",
  "timestamp": 1640995200,
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

### Метрики Prometheus

**Запрос:**
```bash
curl http://localhost:8080/metrics
```

**Ответ:**
```
# HELP fileserver_requests_total Total number of HTTP requests
# TYPE fileserver_requests_total counter
fileserver_requests_total{method="GET",endpoint="/files"} 1234
fileserver_requests_total{method="POST",endpoint="/files/upload"} 567

# HELP fileserver_request_duration_seconds Request duration in seconds
# TYPE fileserver_request_duration_seconds histogram
fileserver_request_duration_seconds_bucket{le="0.1"} 100
fileserver_request_duration_seconds_bucket{le="0.5"} 200
fileserver_request_duration_seconds_bucket{le="1.0"} 250
fileserver_request_duration_seconds_bucket{le="+Inf"} 300

# HELP fileserver_file_operations_total Total number of file operations
# TYPE fileserver_file_operations_total counter
fileserver_file_operations_total{operation="upload"} 567
fileserver_file_operations_total{operation="download"} 890
fileserver_file_operations_total{operation="delete"} 123
```

## Обработка ошибок

### Ошибка аутентификации

**Запрос без токена:**
```bash
curl http://localhost:8080/api/v1/files
```

**Ответ:**
```json
{
  "success": false,
  "error": "Authentication required",
  "code": "UNAUTHORIZED",
  "timestamp": "2024-01-01T12:00:00Z"
}
```

### Файл не найден

**Запрос:**
```bash
curl -H "Authorization: Bearer <token>" \
  http://localhost:8080/api/v1/files/download/nonexistent/file.pdf
```

**Ответ:**
```json
{
  "success": false,
  "error": "File not found",
  "code": "FILE_NOT_FOUND",
  "details": {
    "path": "/nonexistent/file.pdf"
  },
  "timestamp": "2024-01-01T12:00:00Z"
}
```

### Превышение лимита размера файла

**Ответ:**
```json
{
  "success": false,
  "error": "File too large",
  "code": "FILE_TOO_LARGE",
  "details": {
    "max_size_mb": 100,
    "actual_size_mb": 150
  },
  "timestamp": "2024-01-01T12:00:00Z"
}
```

### Неподдерживаемый тип файла

**Ответ:**
```json
{
  "success": false,
  "error": "Unsupported file type",
  "code": "UNSUPPORTED_FILE_TYPE",
  "details": {
    "mime_type": "application/x-executable",
    "allowed_types": [
      "application/pdf",
      "image/jpeg",
      "image/png",
      "text/plain"
    ]
  },
  "timestamp": "2024-01-01T12:00:00Z"
}
```

## Примеры с JavaScript (fetch API)

### Аутентификация

```javascript
async function login(username, password) {
  const response = await fetch('http://localhost:8080/api/v1/auth/login', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({ username, password }),
  });
  
  const data = await response.json();
  
  if (data.success) {
    localStorage.setItem('token', data.token);
    return data.token;
  } else {
    throw new Error(data.error);
  }
}
```

### Загрузка файла

```javascript
async function uploadFile(file, path) {
  const token = localStorage.getItem('token');
  const formData = new FormData();
  formData.append('file', file);
  formData.append('path', path);
  
  const response = await fetch('http://localhost:8080/api/v1/files/upload', {
    method: 'POST',
    headers: {
      'Authorization': `Bearer ${token}`,
    },
    body: formData,
  });
  
  return await response.json();
}
```

### Получение списка файлов

```javascript
async function getFiles(path = '', page = 1, limit = 50) {
  const token = localStorage.getItem('token');
  const params = new URLSearchParams({
    path,
    page: page.toString(),
    limit: limit.toString(),
  });
  
  const response = await fetch(`http://localhost:8080/api/v1/files?${params}`, {
    headers: {
      'Authorization': `Bearer ${token}`,
    },
  });
  
  return await response.json();
}
```

## Примеры с Python (requests)

### Аутентификация

```python
import requests
import json

def login(username, password):
    url = 'http://localhost:8080/api/v1/auth/login'
    data = {
        'username': username,
        'password': password
    }
    
    response = requests.post(url, json=data)
    result = response.json()
    
    if result['success']:
        return result['token']
    else:
        raise Exception(result['error'])
```

### Загрузка файла

```python
def upload_file(token, file_path, remote_path):
    url = 'http://localhost:8080/api/v1/files/upload'
    headers = {
        'Authorization': f'Bearer {token}'
    }
    
    with open(file_path, 'rb') as f:
        files = {'file': f}
        data = {'path': remote_path}
        
        response = requests.post(url, headers=headers, files=files, data=data)
        return response.json()
```

### Скачивание файла

```python
def download_file(token, remote_path, local_path):
    url = f'http://localhost:8080/api/v1/files/download/{remote_path}'
    headers = {
        'Authorization': f'Bearer {token}'
    }
    
    response = requests.get(url, headers=headers)
    
    if response.status_code == 200:
        with open(local_path, 'wb') as f:
            f.write(response.content)
        return True
    else:
        return False
```
