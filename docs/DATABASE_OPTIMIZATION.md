# Database Connection Optimization

## Реализованные оптимизации

### 1. **Connection Pool с защитой от race conditions** ✅

#### Проблема (было):
```cpp
if (stats_.total_connections < config_.max_connections) {
    lock.unlock();
    auto new_conn = createConnection();
    lock.lock();
    stats_.total_connections++;  // Race condition!
}
```

#### Решение (стало):
```cpp
if (stats_.total_connections < config_.max_connections) {
    stats_.total_connections++;  // Резервируем слот ДО создания
    lock.unlock();
    auto new_conn = createConnection();
    lock.lock();
    if (!new_conn) {
        stats_.total_connections--;  // Откатываем при неудаче
    }
}
```

**Best Practice**: Всегда резервируйте ресурсы ДО выполнения долгих операций вне блокировки.

---

### 2. **Устранение утечек памяти в PQexec** ✅

#### Проблема (было):
```cpp
PQexec(pg_conn, "SET client_encoding = 'UTF8'");  // Утечка!
PQexec(pg_conn, "SET timezone = 'UTC'");          // Утечка!
```

#### Решение (стало):
```cpp
PGresult* result = PQexec(pg_conn, "SET client_encoding = 'UTF8'");
if (result) PQclear(result);

result = PQexec(pg_conn, "SET timezone = 'UTC'");
if (result) PQclear(result);
```

**Best Practice**: Всегда вызывайте `PQclear()` для каждого `PQexec()` результата.

---

### 3. **Transaction Support с RAII** ✅

#### Использование:
```cpp
auto conn = pool_->getConnection();
database::Transaction transaction(conn.get());

// Выполняем операции
PGresult* result = PQexec(conn->get(), "UPDATE ...");
if (result) PQclear(result);

if (success) {
    transaction.commit();  // Явный commit
} else {
    // Автоматический rollback при выходе из scope
}
```

**Best Practice**: 
- Используйте транзакции для атомарности связанных операций
- RAII гарантирует rollback при исключениях
- Явно вызывайте `commit()` при успехе

---

### 4. **Row-level Locking для предотвращения race conditions** ✅

#### Пример (authenticate):
```cpp
const char* query = R"(
    SELECT * FROM users
    WHERE username = $1
    FOR UPDATE  -- Блокируем строку!
)";
```

**Best Practice**: 
- Используйте `FOR UPDATE` когда планируете изменять строку
- Это предотвращает lost updates и phantom reads
- Всегда выполняйте в транзакции

---

### 5. **Connection Validation перед возвратом в pool** ✅

#### Реализация:
```cpp
void ConnectionPool::returnConnection(std::unique_ptr<Connection> conn) {
    // Health check
    PGresult* result = PQexec(conn->get(), "SELECT 1");
    bool is_healthy = result && PQresultStatus(result) == PGRES_TUPLES_OK;
    if (result) PQclear(result);
    
    if (is_healthy) {
        // Reset transaction state
        result = PQexec(conn->get(), "ROLLBACK");
        if (result) PQclear(result);
        
        available_connections_.push(std::move(conn));
    } else {
        // Discard broken connection
        stats_.total_connections--;
    }
}
```

**Best Practice**: 
- Всегда валидируйте соединение перед возвратом
- Сбрасывайте транзакционное состояние
- Удаляйте сломанные соединения

---

### 6. **Prepared Statements Support** ✅

#### Использование:
```cpp
auto conn = pool_->getConnection();
database::PreparedStatement stmt(
    conn.get(), 
    "get_user", 
    "SELECT * FROM users WHERE username = $1"
);

std::vector<std::string> params = {username};
PGresult* result = stmt.execute(params);
if (result) {
    // Обработка
    PQclear(result);
}
```

**Best Practice**:
- Используйте prepared statements для часто выполняемых запросов
- Защита от SQL-инъекций
- Повышение производительности (~20-30%)

---

## Best Practices Summary

### 1. Thread Safety
- ✅ Используйте `std::mutex` для защиты shared state
- ✅ Резервируйте ресурсы ДО освобождения блокировки
- ✅ Минимизируйте критические секции

### 2. Resource Management
- ✅ Используйте RAII для автоматической очистки
- ✅ Всегда вызывайте `PQclear()` для результатов
- ✅ Валидируйте соединения при возврате в pool

### 3. Transaction Control
- ✅ Используйте транзакции для атомарных операций
- ✅ Явно вызывайте `commit()` при успехе
- ✅ Полагайтесь на RAII для автоматического rollback

### 4. Performance
- ✅ Используйте connection pooling
- ✅ Prepared statements для частых запросов
- ✅ Row-level locking вместо table-level
- ✅ Batch operations где возможно

### 5. Error Handling
- ✅ Проверяйте все возвращаемые значения
- ✅ Логируйте ошибки для debugging
- ✅ Graceful degradation при недоступности БД

---

## Configuration Recommendations

### Оптимальные настройки ConnectionPool:

```json
{
  "database": {
    "min_connections": 5,       // Минимальный пул для быстрого старта
    "max_connections": 20,      // Зависит от нагрузки
    "connection_timeout": 30,    // Таймаут подключения (секунды)
    "idle_timeout": 300,         // Время жизни idle соединения (5 мин)
    "enable_ssl": true          // Всегда включайте в production
  }
}
```

### PostgreSQL настройки:

```sql
-- Connection pooling
max_connections = 100

-- Statement timeouts
statement_timeout = '30s'
idle_in_transaction_session_timeout = '60s'

-- Performance
shared_buffers = '256MB'
effective_cache_size = '1GB'
work_mem = '16MB'

-- Logging
log_min_duration_statement = 1000  -- Log queries > 1s
```

---

## Monitoring

### Метрики для отслеживания:

```cpp
auto stats = pool->getStats();
// stats.total_connections    - Всего соединений
// stats.active_connections   - Активных сейчас
// stats.idle_connections     - Свободных
// stats.failed_connections   - Количество ошибок
// stats.total_requests       - Всего запросов
// stats.failed_requests      - Неудачных запросов
```

### Алерты:
- ⚠️ `failed_connections` растет → Проблемы с БД
- ⚠️ `active_connections >= max_connections` → Нужно увеличить pool
- ⚠️ `failed_requests > 1%` → Проверить таймауты

---

## Testing Race Conditions

### Stress Test Example:

```bash
# Запустить 100 параллельных запросов
for i in {1..100}; do
    curl -X POST http://localhost:8080/api/v1/auth/login \
         -d '{"username":"user","password":"pass"}' &
done
wait

# Проверить consistency
psql -c "SELECT COUNT(*) FROM user_sessions WHERE is_active = true"
```

---

## Migration Checklist

При развертывании:
- [ ] Создать индексы для часто используемых полей
- [ ] Настроить connection pooling
- [ ] Включить SSL для production
- [ ] Настроить мониторинг метрик
- [ ] Провести stress testing
- [ ] Настроить backup стратегию
- [ ] Документировать query performance

---

**Автор**: FileServer Development Team  
**Дата**: 2024-11-09  
**Версия**: 1.0

