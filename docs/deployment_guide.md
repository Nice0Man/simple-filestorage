# Руководство по развертыванию

## Обзор

Данное руководство описывает различные способы развертывания FileServer в производственной среде.

## Системные требования

### Минимальные требования
- **CPU:** 2 ядра
- **RAM:** 2 GB
- **Диск:** 10 GB свободного места
- **ОС:** Ubuntu 20.04+, CentOS 8+, или аналогичная

### Рекомендуемые требования
- **CPU:** 4+ ядра
- **RAM:** 8+ GB
- **Диск:** 100+ GB SSD
- **ОС:** Ubuntu 22.04 LTS

### Зависимости
- Docker 20.10+
- Docker Compose 2.0+
- Git
- curl (для health checks)

## Способы развертывания

### 1. Docker Compose (Рекомендуемый)

#### Быстрый старт

```bash
# Клонирование репозитория
git clone https://github.com/your-org/cpp-fileserver.git
cd cpp-fileserver

# Создание необходимых директорий
mkdir -p files logs config nginx/ssl monitoring

# Копирование конфигурации
cp config.json config/config.json

# Запуск сервисов
docker-compose up -d
```

#### Проверка состояния

```bash
# Проверка статуса контейнеров
docker-compose ps

# Просмотр логов
docker-compose logs -f fileserver

# Проверка health check
curl http://localhost:8080/health
```

### 2. Standalone Docker

```bash
# Сборка образа
docker build -t fileserver:latest .

# Запуск контейнера
docker run -d \
  --name fileserver \
  -p 8080:8080 \
  -v $(pwd)/files:/app/files \
  -v $(pwd)/logs:/app/logs \
  -v $(pwd)/config:/app/config \
  -e CONFIG_PATH=/app/config/config.json \
  fileserver:latest
```

### 3. Kubernetes

#### Namespace

```yaml
# namespace.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: fileserver
```

#### ConfigMap

```yaml
# configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: fileserver-config
  namespace: fileserver
data:
  config.json: |
    {
      "server": {
        "host": "0.0.0.0",
        "port": 8080
      },
      "storage": {
        "root_directory": "/app/files"
      },
      "logging": {
        "log_file": "/app/logs/fileserver.log",
        "log_level": "INFO"
      }
    }
```

#### Deployment

```yaml
# deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: fileserver
  namespace: fileserver
spec:
  replicas: 3
  selector:
    matchLabels:
      app: fileserver
  template:
    metadata:
      labels:
        app: fileserver
    spec:
      containers:
      - name: fileserver
        image: fileserver:latest
        ports:
        - containerPort: 8080
        env:
        - name: CONFIG_PATH
          value: "/app/config/config.json"
        volumeMounts:
        - name: config
          mountPath: /app/config
        - name: files
          mountPath: /app/files
        - name: logs
          mountPath: /app/logs
        livenessProbe:
          httpGet:
            path: /health
            port: 8080
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /health
            port: 8080
          initialDelaySeconds: 5
          periodSeconds: 5
        resources:
          requests:
            memory: "256Mi"
            cpu: "250m"
          limits:
            memory: "512Mi"
            cpu: "500m"
      volumes:
      - name: config
        configMap:
          name: fileserver-config
      - name: files
        persistentVolumeClaim:
          claimName: fileserver-files
      - name: logs
        emptyDir: {}
```

#### Service

```yaml
# service.yaml
apiVersion: v1
kind: Service
metadata:
  name: fileserver-service
  namespace: fileserver
spec:
  selector:
    app: fileserver
  ports:
  - protocol: TCP
    port: 80
    targetPort: 8080
  type: ClusterIP
```

#### Ingress

```yaml
# ingress.yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: fileserver-ingress
  namespace: fileserver
  annotations:
    nginx.ingress.kubernetes.io/rewrite-target: /
    nginx.ingress.kubernetes.io/ssl-redirect: "true"
    cert-manager.io/cluster-issuer: "letsencrypt-prod"
spec:
  tls:
  - hosts:
    - files.yourdomain.com
    secretName: fileserver-tls
  rules:
  - host: files.yourdomain.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: fileserver-service
            port:
              number: 80
```

#### PersistentVolumeClaim

```yaml
# pvc.yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: fileserver-files
  namespace: fileserver
spec:
  accessModes:
    - ReadWriteMany
  resources:
    requests:
      storage: 100Gi
  storageClassName: nfs-client
```

## Конфигурация

### Основная конфигурация

Отредактируйте `config/config.json`:

```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "max_connections": 1000,
    "timeout_seconds": 30
  },
  "storage": {
    "root_directory": "/app/files",
    "max_file_size_mb": 100,
    "allowed_extensions": [".pdf", ".doc", ".jpg", ".png"]
  },
  "security": {
    "authentication_enabled": true,
    "jwt_secret": "CHANGE_THIS_SECRET_KEY",
    "token_expiry_hours": 24
  },
  "logging": {
    "log_level": "INFO",
    "log_file": "/app/logs/fileserver.log"
  }
}
```

### Переменные окружения

```bash
# Основные настройки
CONFIG_PATH=/app/config/config.json
LOG_LEVEL=INFO

# Безопасность
JWT_SECRET=your-secret-key-here
AUTH_ENABLED=true

# Производительность
MAX_CONNECTIONS=1000
WORKER_THREADS=4

# Мониторинг
METRICS_ENABLED=true
HEALTH_CHECK_ENABLED=true
```

### Nginx конфигурация

Создайте `nginx/nginx.conf`:

```nginx
events {
    worker_connections 1024;
}

http {
    upstream fileserver {
        server fileserver:8080;
    }

    # Rate limiting
    limit_req_zone $binary_remote_addr zone=api:10m rate=10r/s;

    server {
        listen 80;
        server_name localhost;

        # Security headers
        add_header X-Frame-Options DENY;
        add_header X-Content-Type-Options nosniff;
        add_header X-XSS-Protection "1; mode=block";

        # Rate limiting
        limit_req zone=api burst=20 nodelay;

        # File upload size limit
        client_max_body_size 100M;

        location / {
            proxy_pass http://fileserver;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
            
            # Timeouts
            proxy_connect_timeout 60s;
            proxy_send_timeout 60s;
            proxy_read_timeout 60s;
        }

        # Health check endpoint
        location /health {
            proxy_pass http://fileserver/health;
            access_log off;
        }

        # Static files (if needed)
        location /static/ {
            alias /var/www/static/;
            expires 1y;
            add_header Cache-Control "public, immutable";
        }
    }

    # HTTPS server (if SSL certificates are available)
    server {
        listen 443 ssl http2;
        server_name localhost;

        ssl_certificate /etc/nginx/ssl/cert.pem;
        ssl_certificate_key /etc/nginx/ssl/key.pem;
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_ciphers ECDHE-RSA-AES256-GCM-SHA512:DHE-RSA-AES256-GCM-SHA512;
        ssl_prefer_server_ciphers off;

        # Same location blocks as HTTP server
        location / {
            proxy_pass http://fileserver;
            # ... same proxy settings
        }
    }
}
```

## Мониторинг

### Prometheus конфигурация

Создайте `monitoring/prometheus.yml`:

```yaml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

rule_files:
  - "fileserver_rules.yml"

scrape_configs:
  - job_name: 'fileserver'
    static_configs:
      - targets: ['fileserver:8080']
    metrics_path: '/metrics'
    scrape_interval: 10s

  - job_name: 'node-exporter'
    static_configs:
      - targets: ['node-exporter:9100']

alerting:
  alertmanagers:
    - static_configs:
        - targets:
          - alertmanager:9093
```

### Grafana Dashboard

Импортируйте dashboard для мониторинга FileServer:

```json
{
  "dashboard": {
    "title": "FileServer Monitoring",
    "panels": [
      {
        "title": "Request Rate",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(fileserver_requests_total[5m])",
            "legendFormat": "{{method}} {{endpoint}}"
          }
        ]
      },
      {
        "title": "Response Time",
        "type": "graph",
        "targets": [
          {
            "expr": "histogram_quantile(0.95, rate(fileserver_request_duration_seconds_bucket[5m]))",
            "legendFormat": "95th percentile"
          }
        ]
      }
    ]
  }
}
```

## Безопасность

### SSL/TLS сертификаты

#### Let's Encrypt (рекомендуемый)

```bash
# Установка certbot
sudo apt-get install certbot python3-certbot-nginx

# Получение сертификата
sudo certbot --nginx -d files.yourdomain.com

# Автоматическое обновление
sudo crontab -e
# Добавить: 0 12 * * * /usr/bin/certbot renew --quiet
```

#### Самоподписанные сертификаты (для тестирования)

```bash
# Создание сертификата
openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
  -keyout nginx/ssl/key.pem \
  -out nginx/ssl/cert.pem \
  -subj "/C=US/ST=State/L=City/O=Organization/CN=localhost"
```

### Firewall настройки

```bash
# Ubuntu/Debian
sudo ufw allow 22/tcp
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw enable

# CentOS/RHEL
sudo firewall-cmd --permanent --add-service=ssh
sudo firewall-cmd --permanent --add-service=http
sudo firewall-cmd --permanent --add-service=https
sudo firewall-cmd --reload
```

## Резервное копирование

### Автоматическое резервное копирование

Создайте скрипт `backup.sh`:

```bash
#!/bin/bash

BACKUP_DIR="/backups"
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_NAME="fileserver_backup_$DATE"

# Создание директории для бэкапов
mkdir -p $BACKUP_DIR

# Остановка сервиса (опционально)
# docker-compose stop fileserver

# Создание архива файлов
tar -czf "$BACKUP_DIR/$BACKUP_NAME.tar.gz" \
  files/ \
  config/ \
  logs/

# Запуск сервиса
# docker-compose start fileserver

# Удаление старых бэкапов (старше 30 дней)
find $BACKUP_DIR -name "fileserver_backup_*.tar.gz" -mtime +30 -delete

echo "Backup completed: $BACKUP_NAME.tar.gz"
```

### Настройка cron для автоматического бэкапа

```bash
# Редактирование crontab
crontab -e

# Ежедневный бэкап в 2:00
0 2 * * * /path/to/backup.sh >> /var/log/fileserver_backup.log 2>&1
```

## Масштабирование

### Горизонтальное масштабирование

```yaml
# docker-compose.scale.yml
version: '3.8'

services:
  fileserver:
    deploy:
      replicas: 3
    # ... остальная конфигурация

  nginx:
    depends_on:
      - fileserver
    # Load balancing configuration
```

### Вертикальное масштабирование

```yaml
services:
  fileserver:
    deploy:
      resources:
        limits:
          cpus: '2.0'
          memory: 4G
        reservations:
          cpus: '1.0'
          memory: 2G
```

## Troubleshooting

### Общие проблемы

#### Контейнер не запускается

```bash
# Проверка логов
docker-compose logs fileserver

# Проверка конфигурации
docker-compose config

# Проверка портов
netstat -tulpn | grep 8080
```

#### Проблемы с доступом к файлам

```bash
# Проверка прав доступа
ls -la files/
sudo chown -R 1000:1000 files/

# Проверка SELinux (CentOS/RHEL)
sudo setsebool -P httpd_can_network_connect 1
```

#### Высокое потребление памяти

```bash
# Мониторинг ресурсов
docker stats fileserver

# Настройка лимитов в docker-compose.yml
services:
  fileserver:
    mem_limit: 512m
    memswap_limit: 512m
```

### Логи и диагностика

```bash
# Просмотр логов приложения
tail -f logs/fileserver.log

# Системные логи
journalctl -u docker
journalctl -f

# Мониторинг производительности
htop
iotop
```

## Обновление

### Обновление Docker образа

```bash
# Остановка сервисов
docker-compose down

# Обновление образа
docker-compose pull

# Запуск с новым образом
docker-compose up -d

# Проверка статуса
docker-compose ps
curl http://localhost:8080/health
```

### Rolling update в Kubernetes

```bash
# Обновление deployment
kubectl set image deployment/fileserver fileserver=fileserver:v2.0.0 -n fileserver

# Проверка статуса обновления
kubectl rollout status deployment/fileserver -n fileserver

# Откат при необходимости
kubectl rollout undo deployment/fileserver -n fileserver
```
