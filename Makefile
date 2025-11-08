# FileServer Makefile
# Provides convenient commands for building, testing, and containerization

.PHONY: all clean build rebuild test install docker-build docker-run docker-stop docker-clean help

# Configuration
BUILD_DIR := build
BIN_DIR := bin
CMAKE := cmake
CTEST := ctest
DOCKER_COMPOSE := docker-compose
DOCKER := docker
PROJECT_NAME := fileserver
NPROC := $(shell nproc)

# Colors for output
COLOR_RESET := \033[0m
COLOR_BOLD := \033[1m
COLOR_GREEN := \033[32m
COLOR_YELLOW := \033[33m
COLOR_BLUE := \033[34m

# Default target
all: build

# Help target
help:
	@echo "$(COLOR_BOLD)FileServer Build System$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_GREEN)Build Commands:$(COLOR_RESET)"
	@echo "  make build          - Build the project (default)"
	@echo "  make rebuild        - Clean and rebuild the project"
	@echo "  make clean          - Remove build artifacts"
	@echo "  make install        - Install the binary to system"
	@echo ""
	@echo "$(COLOR_GREEN)Testing Commands:$(COLOR_RESET)"
	@echo "  make test           - Run all tests"
	@echo "  make test-verbose   - Run tests with verbose output"
	@echo "  make test-debug     - Run tests with debug build"
	@echo ""
	@echo "$(COLOR_GREEN)Docker Commands:$(COLOR_RESET)"
	@echo "  make docker-build   - Build Docker image"
	@echo "  make docker-build-local - Build Docker image with local binary"
	@echo "  make docker-run     - Run Docker containers"
	@echo "  make docker-stop    - Stop Docker containers"
	@echo "  make docker-clean   - Remove Docker containers and images"
	@echo "  make docker-logs    - Show Docker logs"
	@echo ""
	@echo "$(COLOR_GREEN)Development Commands:$(COLOR_RESET)"
	@echo "  make format         - Format code with clang-format"
	@echo "  make lint           - Run static analysis"
	@echo "  make dev            - Build in debug mode"
	@echo "  make release        - Build in release mode"

# Configure CMake (Release build)
configure:
	@echo "$(COLOR_BLUE)Configuring CMake...$(COLOR_RESET)"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Configure CMake (Debug build)
configure-debug:
	@echo "$(COLOR_BLUE)Configuring CMake (Debug)...$(COLOR_RESET)"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
build: configure
	@echo "$(COLOR_BLUE)Building project...$(COLOR_RESET)"
	@cd $(BUILD_DIR) && $(MAKE) -j$(NPROC)
	@echo "$(COLOR_GREEN)Build complete!$(COLOR_RESET)"

# Build in debug mode
dev: configure-debug
	@echo "$(COLOR_BLUE)Building project (Debug)...$(COLOR_RESET)"
	@cd $(BUILD_DIR) && $(MAKE) -j$(NPROC)
	@echo "$(COLOR_GREEN)Debug build complete!$(COLOR_RESET)"

# Build in release mode
release: configure
	@echo "$(COLOR_BLUE)Building project (Release)...$(COLOR_RESET)"
	@cd $(BUILD_DIR) && $(MAKE) -j$(NPROC)
	@echo "$(COLOR_GREEN)Release build complete!$(COLOR_RESET)"

# Rebuild from scratch
rebuild: clean build

# Clean build artifacts
clean:
	@echo "$(COLOR_YELLOW)Cleaning build artifacts...$(COLOR_RESET)"
	@rm -rf $(BUILD_DIR)
	@echo "$(COLOR_GREEN)Clean complete!$(COLOR_RESET)"

# Run tests
test: build
	@echo "$(COLOR_BLUE)Running tests...$(COLOR_RESET)"
	@cd $(BUILD_DIR) && $(CTEST) --output-on-failure
	@echo "$(COLOR_GREEN)Tests complete!$(COLOR_RESET)"

# Run tests with verbose output
test-verbose: build
	@echo "$(COLOR_BLUE)Running tests (verbose)...$(COLOR_RESET)"
	@cd $(BUILD_DIR) && $(CTEST) -V

# Run tests in debug mode
test-debug: dev
	@echo "$(COLOR_BLUE)Running tests (debug)...$(COLOR_RESET)"
	@cd $(BUILD_DIR) && $(CTEST) --output-on-failure -V

# Install to system
install: build
	@echo "$(COLOR_BLUE)Installing...$(COLOR_RESET)"
	@cd $(BUILD_DIR) && sudo $(MAKE) install
	@echo "$(COLOR_GREEN)Installation complete!$(COLOR_RESET)"

# Docker build (standard multi-stage)
docker-build:
	@echo "$(COLOR_BLUE)Building Docker image...$(COLOR_RESET)"
	@$(DOCKER) build -t $(PROJECT_NAME):latest \
		--build-arg BUILD_DATE=$(shell date -u +'%Y-%m-%dT%H:%M:%SZ') \
		--build-arg VCS_REF=$(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown") \
		-f Dockerfile .
	@echo "$(COLOR_GREEN)Docker build complete!$(COLOR_RESET)"

# Docker build with local binary
docker-build-local:
	@if [ ! -f $(BUILD_DIR)/bin/$(PROJECT_NAME) ]; then \
		echo "$(COLOR_YELLOW)Binary not found. Building locally first...$(COLOR_RESET)"; \
		$(MAKE) build; \
	else \
		echo "$(COLOR_GREEN)Using existing binary: $(BUILD_DIR)/bin/$(PROJECT_NAME)$(COLOR_RESET)"; \
	fi
	@echo "$(COLOR_BLUE)Building Docker image with local binary...$(COLOR_RESET)"
	@$(DOCKER) build -t $(PROJECT_NAME):local \
		--target production-local \
		--build-arg BUILD_DATE=$(shell date -u +'%Y-%m-%dT%H:%M:%SZ') \
		--build-arg VCS_REF=$(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown") \
		-f Dockerfile .
	@echo "$(COLOR_GREEN)Docker build with local binary complete!$(COLOR_RESET)"

# Run Docker containers
docker-run:
	@echo "$(COLOR_BLUE)Starting Docker containers...$(COLOR_RESET)"
	@cd docker && $(DOCKER_COMPOSE) up -d
	@echo "$(COLOR_GREEN)Docker containers started!$(COLOR_RESET)"

# Stop Docker containers
docker-stop:
	@echo "$(COLOR_YELLOW)Stopping Docker containers...$(COLOR_RESET)"
	@cd docker && $(DOCKER_COMPOSE) down
	@echo "$(COLOR_GREEN)Docker containers stopped!$(COLOR_RESET)"

# Clean Docker containers and images
docker-clean: docker-stop
	@echo "$(COLOR_YELLOW)Removing Docker images...$(COLOR_RESET)"
	@$(DOCKER) rmi $(PROJECT_NAME):latest || true
	@cd docker && $(DOCKER_COMPOSE) down -v --rmi all
	@echo "$(COLOR_GREEN)Docker cleanup complete!$(COLOR_RESET)"

# Show Docker logs
docker-logs:
	@cd docker && $(DOCKER_COMPOSE) logs -f fileserver

# Format code (if clang-format is available)
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "$(COLOR_BLUE)Formatting code...$(COLOR_RESET)"; \
		find src include tests -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i; \
		echo "$(COLOR_GREEN)Code formatting complete!$(COLOR_RESET)"; \
	else \
		echo "$(COLOR_YELLOW)clang-format not found, skipping formatting$(COLOR_RESET)"; \
	fi

# Run static analysis (if clang-tidy is available)
lint:
	@if command -v clang-tidy >/dev/null 2>&1; then \
		echo "$(COLOR_BLUE)Running static analysis...$(COLOR_RESET)"; \
		find src -name '*.cpp' | xargs clang-tidy -p $(BUILD_DIR); \
		echo "$(COLOR_GREEN)Static analysis complete!$(COLOR_RESET)"; \
	else \
		echo "$(COLOR_YELLOW)clang-tidy not found, skipping analysis$(COLOR_RESET)"; \
	fi

# Quick build and test cycle
quick: build test
	@echo "$(COLOR_GREEN)Quick build and test complete!$(COLOR_RESET)"

# Development cycle: clean, build, test
dev-cycle: clean dev test-debug
	@echo "$(COLOR_GREEN)Development cycle complete!$(COLOR_RESET)"

# Production build and Docker image
production: clean release docker-build-local
	@echo "$(COLOR_GREEN)Production build complete!$(COLOR_RESET)"

