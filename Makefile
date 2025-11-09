# FileServer Makefile
# Docker-first build system

.PHONY: help up down restart logs clean build-images rebuild status
.PHONY: backend-logs frontend-logs db-logs db-shell setup stop

# Configuration
PROJECT_NAME := fileserver
DOCKER_COMPOSE := docker compose
DOCKER := docker

# Colors for output
COLOR_RESET := \033[0m
COLOR_BOLD := \033[1m
COLOR_GREEN := \033[32m
COLOR_YELLOW := \033[33m
COLOR_BLUE := \033[34m
COLOR_RED := \033[31m

# Default target
.DEFAULT_GOAL := help

# Help target
help:
	@echo "$(COLOR_BOLD)FileServer - Docker Build System$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_GREEN)Main Commands:$(COLOR_RESET)"
	@echo "  make up             - Build and start all services (backend, frontend, db)"
	@echo "  make down           - Stop and remove all containers"
	@echo "  make restart        - Restart all services"
	@echo "  make logs           - Show logs from all services"
	@echo "  make status         - Show status of all services"
	@echo "  make clean          - Remove all containers, images, and volumes"
	@echo ""
	@echo "$(COLOR_GREEN)Build Commands:$(COLOR_RESET)"
	@echo "  make build-images   - Build all Docker images"
	@echo "  make rebuild        - Rebuild and restart all services"
	@echo ""
	@echo "$(COLOR_GREEN)Service-Specific Commands:$(COLOR_RESET)"
	@echo "  make backend-logs   - Show backend logs"
	@echo "  make frontend-logs  - Show frontend logs"
	@echo "  make db-logs        - Show database logs"
	@echo "  make db-shell       - Connect to database shell"
	@echo ""
	@echo "$(COLOR_GREEN)Setup:$(COLOR_RESET)"
	@echo "  make setup          - Initial setup (create directories, .env)"
	@echo ""
	@echo "$(COLOR_BLUE)Services will be available at:$(COLOR_RESET)"
	@echo "  Backend:  http://localhost:8080"
	@echo "  Frontend: http://localhost:80"
	@echo "  Database: localhost:5432"

# Initial setup  
setup:
	@echo "$(COLOR_BLUE)Setting up FileServer...$(COLOR_RESET)"
	@mkdir -p files logs config 2>/dev/null || true
	@if [ ! -f .env ]; then \
		echo "$(COLOR_BLUE)Creating .env file...$(COLOR_RESET)"; \
		echo "# Server Configuration" > .env; \
		echo "SERVER_PORT=8080" >> .env; \
		echo "SERVER_HOST=0.0.0.0" >> .env; \
		echo "" >> .env; \
		echo "# Storage" >> .env; \
		echo "STORAGE_ROOT=/app/files" >> .env; \
		echo "MAX_FILE_SIZE=104857600" >> .env; \
		echo "" >> .env; \
		echo "# JWT" >> .env; \
		echo "JWT_SECRET_KEY=$$(openssl rand -hex 32 2>/dev/null || echo 'change-me-in-production')" >> .env; \
		echo "TOKEN_LIFETIME=24" >> .env; \
		echo "" >> .env; \
		echo "# Database" >> .env; \
		echo "POSTGRES_USER=fileserver" >> .env; \
		echo "POSTGRES_PASSWORD=$$(openssl rand -hex 16 2>/dev/null || echo 'fileserver123')" >> .env; \
		echo "POSTGRES_DB=fileserver" >> .env; \
		echo "DATABASE_URL=postgresql://fileserver:\$$POSTGRES_PASSWORD@db:5432/fileserver" >> .env; \
		echo "" >> .env; \
		echo "# Frontend" >> .env; \
		echo "VITE_API_BASE_URL=http://localhost:8080/api/v1" >> .env; \
		echo "$(COLOR_GREEN)✓ .env created!$(COLOR_RESET)"; \
	else \
		echo "$(COLOR_YELLOW).env already exists, skipping...$(COLOR_RESET)"; \
	fi
	@if [ ! -f config/server.json ] && [ -d config ]; then \
		echo "$(COLOR_BLUE)Creating default server config...$(COLOR_RESET)"; \
		(echo '{' && \
		echo '  "server": {' && \
		echo '    "host": "0.0.0.0",' && \
		echo '    "port": 8080,' && \
		echo '    "max_threads": 8' && \
		echo '  },' && \
		echo '  "storage": {' && \
		echo '    "root_path": "/app/files",' && \
		echo '    "max_file_size": 104857600,' && \
		echo '    "allowed_extensions": ["*"]' && \
		echo '  },' && \
		echo '  "auth": {' && \
		echo '    "token_lifetime_hours": 24' && \
		echo '  },' && \
		echo '  "logging": {' && \
		echo '    "level": "info",' && \
		echo '    "file": "/app/logs/server.log"' && \
		echo '  }' && \
		echo '}') > config/server.json 2>/dev/null && \
		echo "$(COLOR_GREEN)✓ config/server.json created!$(COLOR_RESET)" || \
		echo "$(COLOR_YELLOW)⚠ Could not create config/server.json (will use defaults)$(COLOR_RESET)"; \
	fi
	@echo "$(COLOR_GREEN)✓ Setup complete!$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_YELLOW)Next step: make up$(COLOR_RESET)"

# Build all Docker images
build-images:
	@echo "$(COLOR_BLUE)Building Docker images...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) build --parallel
	@echo "$(COLOR_GREEN)✓ Images built successfully!$(COLOR_RESET)"

# Start all services
up: setup
	@echo "$(COLOR_BLUE)Starting all services...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) up -d --build
	@echo ""
	@echo "$(COLOR_GREEN)✓ Services started!$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_BOLD)Services:$(COLOR_RESET)"
	@echo "  Frontend: $(COLOR_BLUE)http://localhost:80$(COLOR_RESET)"
	@echo "  Backend:  $(COLOR_BLUE)http://localhost:8080$(COLOR_RESET)"
	@echo "  Database: $(COLOR_BLUE)localhost:5432$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_YELLOW)View logs: make logs$(COLOR_RESET)"
	@echo "$(COLOR_YELLOW)Check status: make status$(COLOR_RESET)"

# Stop all services
down:
	@echo "$(COLOR_YELLOW)Stopping all services...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) down
	@echo "$(COLOR_GREEN)✓ Services stopped!$(COLOR_RESET)"

# Stop all services (alias)
stop: down

# Restart all services
restart:
	@echo "$(COLOR_BLUE)Restarting all services...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) restart
	@echo "$(COLOR_GREEN)✓ Services restarted!$(COLOR_RESET)"

# Rebuild and restart
rebuild:
	@echo "$(COLOR_BLUE)Rebuilding and restarting...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) down
	@$(DOCKER_COMPOSE) up -d --build --force-recreate
	@echo "$(COLOR_GREEN)✓ Rebuild complete!$(COLOR_RESET)"

# Show logs from all services
logs:
	@$(DOCKER_COMPOSE) logs -f

# Show backend logs
backend-logs:
	@$(DOCKER_COMPOSE) logs -f server

# Show frontend logs
frontend-logs:
	@$(DOCKER_COMPOSE) logs -f client

# Show database logs
db-logs:
	@$(DOCKER_COMPOSE) logs -f db

# Show service status
status:
	@echo "$(COLOR_BOLD)Service Status:$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) ps

# Connect to database shell
db-shell:
	@echo "$(COLOR_BLUE)Connecting to PostgreSQL...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) exec db psql -U fileserver -d fileserver

# Clean everything
clean:
	@echo "$(COLOR_RED)Warning: This will remove all containers, images, and volumes!$(COLOR_RESET)"
	@echo -n "Are you sure? [y/N] " && read ans && [ $${ans:-N} = y ]
	@echo "$(COLOR_YELLOW)Cleaning up...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) down -v --rmi all --remove-orphans
	@rm -rf files/* logs/* 2>/dev/null || true
	@echo "$(COLOR_GREEN)✓ Cleanup complete!$(COLOR_RESET)"

# Quick cleanup (keep images)
clean-containers:
	@echo "$(COLOR_YELLOW)Removing containers and volumes...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) down -v --remove-orphans
	@echo "$(COLOR_GREEN)✓ Containers removed!$(COLOR_RESET)"

# Backup database
backup-db:
	@echo "$(COLOR_BLUE)Backing up database...$(COLOR_RESET)"
	@mkdir -p backups
	@$(DOCKER_COMPOSE) exec -T db pg_dump -U fileserver fileserver > backups/backup_$$(date +%Y%m%d_%H%M%S).sql
	@echo "$(COLOR_GREEN)✓ Database backed up to backups/$(COLOR_RESET)"

# Restore database
restore-db:
	@echo "$(COLOR_BLUE)Available backups:$(COLOR_RESET)"
	@ls -1 backups/*.sql 2>/dev/null || echo "No backups found"
	@echo ""
	@echo "Usage: make restore-db FILE=backups/backup_YYYYMMDD_HHMMSS.sql"

# Show resource usage
stats:
	@echo "$(COLOR_BOLD)Resource Usage:$(COLOR_RESET)"
	@$(DOCKER) stats --no-stream $$($(DOCKER_COMPOSE) ps -q)

# View backend config
show-config:
	@echo "$(COLOR_BOLD)Server Configuration:$(COLOR_RESET)"
	@cat config/server.json 2>/dev/null || echo "Config file not found. Run 'make setup' first."

# View environment
show-env:
	@echo "$(COLOR_BOLD)Environment Variables:$(COLOR_RESET)"
	@cat .env 2>/dev/null || echo ".env file not found. Run 'make setup' first."

# Health check
health:
	@echo "$(COLOR_BOLD)Health Check:$(COLOR_RESET)"
	@echo -n "Backend:  "
	@curl -sf http://localhost:8080/health >/dev/null && echo "$(COLOR_GREEN)✓ OK$(COLOR_RESET)" || echo "$(COLOR_RED)✗ FAIL$(COLOR_RESET)"
	@echo -n "Frontend: "
	@curl -sf http://localhost:80/health >/dev/null && echo "$(COLOR_GREEN)✓ OK$(COLOR_RESET)" || echo "$(COLOR_RED)✗ FAIL$(COLOR_RESET)"
	@echo -n "Database: "
	@$(DOCKER_COMPOSE) exec -T db pg_isready -U fileserver >/dev/null 2>&1 && echo "$(COLOR_GREEN)✓ OK$(COLOR_RESET)" || echo "$(COLOR_RED)✗ FAIL$(COLOR_RESET)"

# Development helpers
dev-backend:
	@echo "$(COLOR_BLUE)Opening backend shell...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) exec server /bin/sh

dev-frontend:
	@echo "$(COLOR_BLUE)Opening frontend shell...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) exec client /bin/sh

# Update images
update:
	@echo "$(COLOR_BLUE)Updating images...$(COLOR_RESET)"
	@$(DOCKER_COMPOSE) pull
	@echo "$(COLOR_GREEN)✓ Images updated!$(COLOR_RESET)"

# Show all containers (including stopped)
ps-all:
	@$(DOCKER_COMPOSE) ps -a

# Print environment info
info:
	@echo "$(COLOR_BOLD)Environment Information:$(COLOR_RESET)"
	@echo "  Project:        $(PROJECT_NAME)"
	@echo "  Docker:         $$($(DOCKER) --version 2>/dev/null || echo 'not found')"
	@echo "  Docker Compose: $$($(DOCKER_COMPOSE) version --short 2>/dev/null || echo 'not found')"
	@echo "  Git:            $$(git describe --tags 2>/dev/null || git rev-parse --short HEAD 2>/dev/null || echo 'not a git repo')"
	@echo ""
	@echo "$(COLOR_BOLD)Configuration Files:$(COLOR_RESET)"
	@ls -1 docker-compose.yml .env config/server.json 2>/dev/null || echo "  Some config files are missing. Run 'make setup'"

# Follow logs of specific service
logs-follow:
	@echo "Usage: make logs-follow SERVICE=server|client|db"
	@if [ -n "$(SERVICE)" ]; then \
		$(DOCKER_COMPOSE) logs -f $(SERVICE); \
	fi
