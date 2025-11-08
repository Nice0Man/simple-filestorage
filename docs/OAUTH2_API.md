# OAuth2 Authentication API

## Обзор

FileServer реализует OAuth2-подобную систему аутентификации с использованием JWT токенов. Все endpoints следуют стандартным практикам OAuth2 и REST API.

## Base URL

```
http://localhost:8080/api/v1/auth
```

## Endpoints

### 1. Регистрация пользователя

**POST** `/api/v1/auth/register`

Создает нового пользователя и возвращает access token.

#### Request Body:

```json
{
  "username": "string",      // required, минимум 3 символа
  "password": "string",      // required, минимум 6 символов
  "role": "string"          // optional, по умолчанию "user" (user|admin)
}
```

#### Success Response (201 Created):

```json
{
  "message": "User registered successfully",
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "token_type": "Bearer",
  "expires_in": 3600,
  "user": {
    "username": "john_doe",
    "role": "user"
  }
}
```

#### Error Responses:

**400 Bad Request** - Невалидные данные:
```json
{
  "error": "Invalid username",
  "message": "Username must be at least 3 characters long"
}
```

**409 Conflict** - Пользователь уже существует:
```json
{
  "error": "Registration failed",
  "message": "Username already exists"
}
```

#### cURL Example:

```bash
curl -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "john_doe",
    "password": "secure_password",
    "role": "user"
  }'
```

---

### 2. Вход (Login)

**POST** `/api/v1/auth/login`

Аутентифицирует пользователя и возвращает access token.

#### Request Body:

```json
{
  "username": "string",      // required
  "password": "string"       // required
}
```

#### Success Response (200 OK):

```json
{
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "token_type": "Bearer",
  "expires_in": 3600,
  "user": {
    "username": "john_doe",
    "role": "user"
  }
}
```

#### Error Response (401 Unauthorized):

```json
{
  "error": "Authentication failed",
  "message": "Invalid username or password"
}
```

#### cURL Example:

```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "john_doe",
    "password": "secure_password"
  }'
```

---

### 3. Обновление токена (Refresh Token)

**POST** `/api/v1/auth/refresh`

Обновляет access token перед его истечением.

#### Headers:

```
Authorization: Bearer <current_access_token>
```

#### Success Response (200 OK):

```json
{
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "token_type": "Bearer",
  "expires_in": 3600
}
```

#### Error Responses:

**401 Unauthorized** - Невалидный или истекший токен:
```json
{
  "error": "Invalid token",
  "message": "Token is expired or invalid"
}
```

#### cURL Example:

```bash
curl -X POST http://localhost:8080/api/v1/auth/refresh \
  -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
```

---

### 4. Выход (Logout)

**POST** `/api/v1/auth/logout`

Отзывает (инвалидирует) access token.

#### Headers:

```
Authorization: Bearer <access_token>
```

#### Success Response (200 OK):

```json
{
  "message": "Logged out successfully"
}
```

#### cURL Example:

```bash
curl -X POST http://localhost:8080/api/v1/auth/logout \
  -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
```

---

### 5. Получить информацию о текущем пользователе

**GET** `/api/v1/auth/me`

Возвращает информацию о пользователе, связанном с токеном.

#### Headers:

```
Authorization: Bearer <access_token>
```

#### Success Response (200 OK):

```json
{
  "username": "john_doe",
  "role": "user",
  "is_active": true,
  "created_at": 1699459200,
  "last_login": 1699545600
}
```

#### Error Response (404 Not Found):

```json
{
  "error": "User not found",
  "message": "Invalid or expired token"
}
```

#### cURL Example:

```bash
curl -X GET http://localhost:8080/api/v1/auth/me \
  -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
```

---

## Использование токенов

После успешной аутентификации вы получаете `access_token`. Используйте его для авторизованных запросов:

```
Authorization: Bearer <access_token>
```

### Примеры защищенных endpoints:

```bash
# Загрузка файла
curl -X POST http://localhost:8080/api/v1/files/upload \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -F "file=@document.pdf"

# Список файлов
curl -X GET http://localhost:8080/api/v1/files \
  -H "Authorization: Bearer YOUR_TOKEN"

# Скачивание файла
curl -X GET http://localhost:8080/api/v1/files/download/document.pdf \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -O

# Удаление файла
curl -X DELETE http://localhost:8080/api/v1/files/document.pdf \
  -H "Authorization: Bearer YOUR_TOKEN"
```

---

## Публичные endpoints (не требуют аутентификации)

- `GET /health` - Health check
- `POST /api/v1/auth/login` - Вход
- `POST /api/v1/auth/register` - Регистрация
- `GET /docs` - Swagger UI
- `GET /swagger.yaml` - Swagger спецификация

---

## Коды ошибок

| Код | Описание |
|-----|----------|
| 200 | OK - Успешный запрос |
| 201 | Created - Ресурс создан |
| 400 | Bad Request - Невалидные данные |
| 401 | Unauthorized - Требуется аутентификация |
| 404 | Not Found - Ресурс не найден |
| 409 | Conflict - Конфликт (напр., пользователь существует) |
| 500 | Internal Server Error - Ошибка сервера |

---

## Полный пример workflow

### 1. Регистрация нового пользователя:

```bash
# Регистрация
RESPONSE=$(curl -s -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "testpass123"
  }')

# Извлечение токена
TOKEN=$(echo $RESPONSE | jq -r '.access_token')
echo "Token: $TOKEN"
```

### 2. Использование токена для загрузки файла:

```bash
# Загрузка файла
curl -X POST http://localhost:8080/api/v1/files/upload \
  -H "Authorization: Bearer $TOKEN" \
  -F "file=@myfile.txt"
```

### 3. Получение информации о себе:

```bash
curl -X GET http://localhost:8080/api/v1/auth/me \
  -H "Authorization: Bearer $TOKEN" | jq
```

### 4. Выход:

```bash
curl -X POST http://localhost:8080/api/v1/auth/logout \
  -H "Authorization: Bearer $TOKEN"
```

---

## JavaScript/Fetch Example

```javascript
// Регистрация
async function register(username, password) {
  const response = await fetch('http://localhost:8080/api/v1/auth/register', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({ username, password })
  });
  
  const data = await response.json();
  
  if (response.ok) {
    // Сохранить токен
    localStorage.setItem('access_token', data.access_token);
    return data;
  } else {
    throw new Error(data.message);
  }
}

// Вход
async function login(username, password) {
  const response = await fetch('http://localhost:8080/api/v1/auth/login', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({ username, password })
  });
  
  const data = await response.json();
  
  if (response.ok) {
    localStorage.setItem('access_token', data.access_token);
    return data;
  } else {
    throw new Error(data.message);
  }
}

// Получить информацию о пользователе
async function getUserInfo() {
  const token = localStorage.getItem('access_token');
  
  const response = await fetch('http://localhost:8080/api/v1/auth/me', {
    headers: {
      'Authorization': `Bearer ${token}`
    }
  });
  
  if (response.ok) {
    return await response.json();
  } else {
    throw new Error('Failed to get user info');
  }
}

// Выход
async function logout() {
  const token = localStorage.getItem('access_token');
  
  await fetch('http://localhost:8080/api/v1/auth/logout', {
    method: 'POST',
    headers: {
      'Authorization': `Bearer ${token}`
    }
  });
  
  localStorage.removeItem('access_token');
}

// Утилита для авторизованных запросов
async function authenticatedRequest(url, options = {}) {
  const token = localStorage.getItem('access_token');
  
  const response = await fetch(url, {
    ...options,
    headers: {
      ...options.headers,
      'Authorization': `Bearer ${token}`
    }
  });
  
  if (response.status === 401) {
    // Токен истек, перенаправить на login
    window.location.href = '/login';
    return null;
  }
  
  return response;
}
```

---

## Python Example

```python
import requests
import json

class FileServerAuth:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.token = None
    
    def register(self, username, password, role="user"):
        """Регистрация нового пользователя"""
        url = f"{self.base_url}/api/v1/auth/register"
        data = {
            "username": username,
            "password": password,
            "role": role
        }
        
        response = requests.post(url, json=data)
        
        if response.status_code == 201:
            result = response.json()
            self.token = result['access_token']
            return result
        else:
            raise Exception(f"Registration failed: {response.json()}")
    
    def login(self, username, password):
        """Вход в систему"""
        url = f"{self.base_url}/api/v1/auth/login"
        data = {
            "username": username,
            "password": password
        }
        
        response = requests.post(url, json=data)
        
        if response.status_code == 200:
            result = response.json()
            self.token = result['access_token']
            return result
        else:
            raise Exception(f"Login failed: {response.json()}")
    
    def get_user_info(self):
        """Получить информацию о текущем пользователе"""
        if not self.token:
            raise Exception("Not authenticated")
        
        url = f"{self.base_url}/api/v1/auth/me"
        headers = {"Authorization": f"Bearer {self.token}"}
        
        response = requests.get(url, headers=headers)
        
        if response.status_code == 200:
            return response.json()
        else:
            raise Exception(f"Failed to get user info: {response.json()}")
    
    def logout(self):
        """Выход из системы"""
        if not self.token:
            return
        
        url = f"{self.base_url}/api/v1/auth/logout"
        headers = {"Authorization": f"Bearer {self.token}"}
        
        requests.post(url, headers=headers)
        self.token = None
    
    def authenticated_request(self, method, endpoint, **kwargs):
        """Выполнить авторизованный запрос"""
        if not self.token:
            raise Exception("Not authenticated")
        
        url = f"{self.base_url}{endpoint}"
        headers = kwargs.get('headers', {})
        headers['Authorization'] = f"Bearer {self.token}"
        kwargs['headers'] = headers
        
        return requests.request(method, url, **kwargs)

# Пример использования
if __name__ == "__main__":
    auth = FileServerAuth()
    
    # Регистрация
    try:
        result = auth.register("testuser", "testpass123")
        print(f"Registered: {result['user']['username']}")
    except Exception as e:
        print(f"Registration error: {e}")
        # Попробовать войти
        result = auth.login("testuser", "testpass123")
        print(f"Logged in: {result['user']['username']}")
    
    # Получить информацию о пользователе
    user_info = auth.get_user_info()
    print(f"User info: {json.dumps(user_info, indent=2)}")
    
    # Загрузить файл (пример)
    # with open('file.txt', 'rb') as f:
    #     files = {'file': f}
    #     response = auth.authenticated_request('POST', '/api/v1/files/upload', files=files)
    #     print(response.json())
    
    # Выход
    auth.logout()
    print("Logged out")
```

---

## Безопасность

### Рекомендации:

1. **HTTPS**: В production всегда используйте HTTPS
2. **Сильные пароли**: Минимум 8-10 символов, включая цифры и спецсимволы
3. **Хранение токенов**: 
   - В браузере: `localStorage` или `sessionStorage`
   - В мобильных приложениях: Secure Storage
   - Никогда не храните в cookies без флага `httpOnly`
4. **Обработка истечения**: Реализуйте автоматическое обновление токенов
5. **Rate Limiting**: Ограничьте количество попыток входа
6. **JWT Secret**: Используйте криптографически стойкий секретный ключ

---

## Troubleshooting

### "Authentication required"
- Убедитесь, что токен передается в заголовке `Authorization`
- Проверьте формат: `Bearer <token>`
- Токен может быть истекшим - попробуйте `/refresh` или войдите заново

### "Invalid username or password"
- Проверьте правильность учетных данных
- Username чувствителен к регистру

### "Username already exists"
- Выберите другое имя пользователя
- Или войдите с существующим

---

## API Limits

- **Token Lifetime**: 3600 секунд (1 час)
- **Max Login Attempts**: 5 (настраивается)
- **Lockout Duration**: 15 минут (настраивается)
- **Username Length**: Минимум 3 символа
- **Password Length**: Минимум 6 символов

---

## Changelog

### v1.0.0 (2025-11-08)
- ✅ POST `/api/v1/auth/register` - Регистрация пользователя
- ✅ POST `/api/v1/auth/login` - Вход
- ✅ POST `/api/v1/auth/refresh` - Обновление токена
- ✅ POST `/api/v1/auth/logout` - Выход
- ✅ GET `/api/v1/auth/me` - Информация о пользователе
- ✅ OAuth2-совместимый формат ответов
- ✅ JWT токены
- ✅ Bearer authentication

