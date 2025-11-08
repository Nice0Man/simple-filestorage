# Архитектура файлового сервера

## Обзор

Файловый сервер представляет собой высокопроизводительное приложение на C++17, построенное с использованием современных паттернов проектирования и лучших практик разработки.

## Архитектурные принципы

### 1. Модульность
- Четкое разделение ответственности между компонентами
- Слабая связанность между модулями
- Высокая когезия внутри модулей

### 2. Расширяемость
- Использование паттерна Strategy для различных операций с файлами
- Паттерн Command для обработки HTTP-запросов
- Возможность легкого добавления новых функций

### 3. Безопасность
- Аутентификация и авторизация пользователей
- Контроль доступа на основе ролей (RBAC)
- Валидация путей для предотвращения атак directory traversal

### 4. Производительность
- Многопоточная обработка запросов
- Эффективное управление памятью
- Оптимизированные алгоритмы работы с файлами

## Основные компоненты

### Core Layer (Основной слой)

#### FileServer (Singleton)
- Главный класс приложения
- Управляет жизненным циклом сервера
- Координирует работу всех компонентов

```cpp
class FileServer {
public:
    static FileServer& getInstance();
    bool initialize(const std::string& config_path);
    bool start();
    void stop();
};
```

#### FileManager (Strategy Pattern)
- Управляет операциями с файлами
- Использует различные стратегии для разных операций
- Обеспечивает безопасность доступа к файлам

```cpp
class FileManager {
public:
    void setStrategy(std::unique_ptr<FileOperationStrategy> strategy);
    bool executeOperation(const std::string& path, const std::string& data);
};
```

#### RequestHandler (Command Pattern + Facade)
- Обрабатывает HTTP-запросы
- Маршрутизирует запросы к соответствующим командам
- Предоставляет единый интерфейс для работы с запросами

### Utils Layer (Слой утилит)

#### Logger (Singleton)
- Потокобезопасное логирование
- Различные уровни логирования
- Ротация логов

#### ConfigManager
- Управление конфигурацией приложения
- Загрузка настроек из JSON-файлов
- Валидация конфигурации

#### MimeTypeDetector
- Определение MIME-типов файлов
- Поддержка пользовательских типов
- Валидация допустимых типов файлов

### Security Layer (Слой безопасности)

#### AuthManager
- Аутентификация пользователей
- Управление JWT-токенами
- Хеширование паролей

#### AccessControl
- Контроль доступа на основе ролей
- Проверка разрешений для ресурсов
- Управление правами доступа

## Паттерны проектирования

### 1. Singleton Pattern
**Применение:** FileServer, Logger
**Цель:** Обеспечить единственный экземпляр критически важных компонентов

```cpp
class FileServer {
private:
    static std::unique_ptr<FileServer> instance_;
    static std::mutex instance_mutex_;
    FileServer() = default;
public:
    static FileServer& getInstance();
};
```

### 2. Strategy Pattern
**Применение:** FileManager с различными стратегиями операций
**Цель:** Инкапсулировать алгоритмы и сделать их взаимозаменяемыми

```cpp
class FileOperationStrategy {
public:
    virtual bool execute(const std::string& path, const std::string& data) = 0;
};

class UploadStrategy : public FileOperationStrategy { /* ... */ };
class DownloadStrategy : public FileOperationStrategy { /* ... */ };
```

### 3. Command Pattern
**Применение:** RequestHandler с командами для различных HTTP-операций
**Цель:** Инкапсулировать запросы как объекты

```cpp
class Command {
public:
    virtual void execute(const HttpRequest& req, HttpResponse& res) = 0;
};

class UploadCommand : public Command { /* ... */ };
class DownloadCommand : public Command { /* ... */ };
```

### 4. Facade Pattern
**Применение:** RequestHandler как фасад для всей системы обработки запросов
**Цель:** Предоставить упрощенный интерфейс к сложной подсистеме

## Диаграмма компонентов

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   FileServer    │────│ RequestHandler  │────│  FileManager    │
│   (Singleton)   │    │   (Facade)      │    │  (Strategy)     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ ConfigManager   │    │    Commands     │    │   Strategies    │
│                 │    │  (Command)      │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     Logger      │    │   AuthManager   │    │ AccessControl   │
│   (Singleton)   │    │                 │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Потоки данных

### 1. Обработка запроса на загрузку файла
```
HTTP Request → RequestHandler → UploadCommand → FileManager → UploadStrategy → File System
```

### 2. Обработка запроса на скачивание файла
```
HTTP Request → RequestHandler → DownloadCommand → FileManager → DownloadStrategy → File System
```

### 3. Аутентификация
```
HTTP Request → RequestHandler → AuthManager → AccessControl → Response
```

## Масштабируемость

### Горизонтальное масштабирование
- Поддержка запуска нескольких экземпляров
- Балансировка нагрузки через reverse proxy
- Shared storage для файлов

### Вертикальное масштабирование
- Многопоточная обработка запросов
- Настраиваемый пул потоков
- Эффективное использование ресурсов

## Мониторинг и метрики

- Интеграция с Prometheus для сбора метрик
- Health check endpoints
- Детальное логирование операций
- Мониторинг производительности

## Безопасность

### Аутентификация
- JWT-токены для авторизации
- Безопасное хранение паролей (bcrypt)
- Настраиваемое время жизни токенов

### Авторизация
- Role-Based Access Control (RBAC)
- Гранулярные разрешения
- Контроль доступа к путям

### Защита от атак
- Валидация путей файлов
- Rate limiting
- CORS protection
- Input sanitization
