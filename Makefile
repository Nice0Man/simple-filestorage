# FileServer Makefile
# Docker-first build system

.PHONY: help up down restart logs clean build-images rebuild status
.PHONY: backend-logs frontend-logs db-logs db-shell setup stop
.PHONY: test valgrind valgrind-full memcheck install-valgrind
.PHONY: build-local build-libs build-release build-debug clean-build
.PHONY: docker-local docker-remote install-local

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
	@echo "$(COLOR_GREEN)Local Build Commands:$(COLOR_RESET)"
	@echo "  make build-local    - Quick incremental build (FAST)"
	@echo "  make build-release  - Full clean release build"
	@echo "  make build-debug    - Build debug version"
	@echo "  make build-libs     - Build only libraries"
	@echo "  make clean-build    - Clean build directory"
	@echo "  make install-local  - Install binary to /usr/local/bin"
	@echo ""
	@echo "$(COLOR_GREEN)Docker Build Commands:$(COLOR_RESET)"
	@echo "  make docker-local       - Docker with existing binary (FASTEST)"
	@echo "  make docker-local-fresh - Docker with fresh build"
	@echo "  make docker-remote      - Docker full remote build (slowest)"
	@echo ""
	@echo "$(COLOR_GREEN)Testing Commands:$(COLOR_RESET)"
	@echo "  make test           - Run all unit tests"
	@echo "  make valgrind       - Run Valgrind memory leak tests"
	@echo "  make valgrind-full  - Full Valgrind check on main binary"
	@echo "  make memcheck       - Alias for valgrind"
	@echo "  make install-valgrind - Install Valgrind tool"
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

# ==============================================================================
# Local Build Commands
# ==============================================================================

# Build locally (release mode) - FAST incremental
build-local:
	@echo "$(COLOR_BLUE)Quick incremental build...$(COLOR_RESET)"
	@./scripts/quick_build.sh
	@echo "$(COLOR_GREEN)✓ Local build completed!$(COLOR_RESET)"
	@echo "Binary: build/bin/fileserver"

# Build release version (full clean build)
build-release:
	@echo "$(COLOR_BLUE)Building Release version (full)...$(COLOR_RESET)"
	@rm -rf build
	@mkdir -p build
	@cd build && \
		cmake -DCMAKE_BUILD_TYPE=Release \
			  -DCMAKE_CXX_FLAGS_RELEASE="-O2 -DNDEBUG" .. && \
		make -j$$(nproc)
	@echo "$(COLOR_GREEN)✓ Release build completed$(COLOR_RESET)"

# Build debug version
build-debug:
	@echo "$(COLOR_BLUE)Building Debug version...$(COLOR_RESET)"
	@mkdir -p build
	@cd build && \
		cmake -DCMAKE_BUILD_TYPE=Debug \
			  -DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -fno-omit-frame-pointer" .. && \
		make -j$$(nproc)
	@echo "$(COLOR_GREEN)✓ Debug build completed$(COLOR_RESET)"

# Build only libraries
build-libs:
	@echo "$(COLOR_BLUE)Building libraries...$(COLOR_RESET)"
	@mkdir -p build
	@cd build && \
		cmake -DCMAKE_BUILD_TYPE=Release .. && \
		make -j$$(nproc) fileserver_utils fileserver_database fileserver_security fileserver_core
	@echo "$(COLOR_GREEN)✓ Libraries built$(COLOR_RESET)"
	@ls -lh build/lib/

# Clean build directory
clean-build:
	@echo "$(COLOR_YELLOW)Cleaning build directory...$(COLOR_RESET)"
	@rm -rf build build_test
	@echo "$(COLOR_GREEN)✓ Build directory cleaned$(COLOR_RESET)"

# Install binary locally
install-local: build-release
	@echo "$(COLOR_BLUE)Installing fileserver...$(COLOR_RESET)"
	@sudo install -m 755 build/bin/fileserver /usr/local/bin/
	@sudo mkdir -p /etc/fileserver
	@sudo cp -n config/server.json /etc/fileserver/config.json 2>/dev/null || true
	@echo "$(COLOR_GREEN)✓ Installed to /usr/local/bin/fileserver$(COLOR_RESET)"

# Docker build with local binary (FAST - uses existing build)
docker-local:
	@echo "$(COLOR_BLUE)Building Docker image with local binary...$(COLOR_RESET)"
	@if [ ! -f "build/bin/fileserver" ]; then \
		echo "$(COLOR_YELLOW)No binary found, building first...$(COLOR_RESET)"; \
		$(MAKE) build-local; \
	fi
	@$(DOCKER) build -f Dockerfile.local -t fileserver:local .
	@echo "$(COLOR_GREEN)✓ Docker image built: fileserver:local$(COLOR_RESET)"

# Docker build with fresh build
docker-local-fresh: build-release docker-local

# Docker build (full remote build)
docker-remote:
	@echo "$(COLOR_BLUE)Building Docker image (full build)...$(COLOR_RESET)"
	@$(DOCKER) build -t fileserver:latest --target final-production .
	@echo "$(COLOR_GREEN)✓ Docker image built: fileserver:latest$(COLOR_RESET)"

# ==============================================================================
# Testing & Memory Check
# ==============================================================================

# Install valgrind
install-valgrind:
	@echo "$(COLOR_BLUE)Installing valgrind...$(COLOR_RESET)"
	@command -v valgrind >/dev/null 2>&1 && echo "$(COLOR_GREEN)✓ Valgrind already installed$(COLOR_RESET)" || \
		(sudo apt-get update && sudo apt-get install -y valgrind && echo "$(COLOR_GREEN)✓ Valgrind installed$(COLOR_RESET)")

# Run all tests
test:
	@echo "$(COLOR_BLUE)Running tests...$(COLOR_RESET)"
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j$$(nproc) && ctest --output-on-failure
	@echo "$(COLOR_GREEN)✓ Tests completed$(COLOR_RESET)"

# Run valgrind memory check on tests
valgrind: install-valgrind
	@echo "$(COLOR_BLUE)Running Valgrind memory leak tests...$(COLOR_RESET)"
	@./scripts/valgrind_test.sh
	@echo "$(COLOR_GREEN)✓ Valgrind tests completed$(COLOR_RESET)"

# Run valgrind with full leak check on main binary
valgrind-full: install-valgrind
	@echo "$(COLOR_BLUE)Running full Valgrind check on main binary...$(COLOR_RESET)"
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -fno-omit-frame-pointer" .. && make -j$$(nproc)
	@./scripts/valgrind_test.sh build/fileserver
	@echo "$(COLOR_GREEN)✓ Full valgrind check completed$(COLOR_RESET)"

# Memory check alias
memcheck: valgrind

# ==============================================================================
# Service Management
# ==============================================================================

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
