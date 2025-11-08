# Simple File Storage

[![CI Status](https://github.com/Nice0Man/simple-filestorage/actions/workflows/ci.yml/badge.svg)](https://github.com/Nice0Man/simple-filestorage/actions/workflows/ci.yml)
[![Test Coverage](https://img.shields.io/badge/tests-73%2F73%20passing-brightgreen)](https://github.com/Nice0Man/simple-filestorage/actions)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A file storage server with RESTful API built in C++17.

[Documentation](docs/) • [API Reference](docs/api_reference.md) • [Architecture](docs/architecture.md)

---

## Overview

Simple File Storage is a file management server that provides HTTP REST API for file operations. The project implements authentication, role-based access control, and basic security measures.

### Key Characteristics

- Multi-threaded HTTP server with concurrent request handling
- JWT-based authentication with role-based access control
- Path validation to prevent directory traversal attacks
- MIME type detection for common file formats
- Docker containerization support
- Automated CI/CD with GitHub Actions
- Test suite with 73 unit and integration tests

---

## Features

### File Operations

- Upload files via multipart/form-data
- Download files with streaming
- Delete files with authorization checks
- List files in storage
- MIME type detection for 50+ file extensions

### Authentication & Authorization

- JWT token-based authentication
- User and Admin role separation
- Password hashing (SHA-256)
- Token expiration management
- Access control for file operations

### Configuration & Deployment

- JSON-based configuration file
- Environment variable support
- Docker deployment
- Automated CI/CD pipeline
- Cross-platform build (Linux, macOS, Windows)

---

## Quick Start

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or higher
- OpenSSL development libraries

### Build from Source

```bash
git clone https://github.com/Nice0Man/simple-filestorage.git
cd simple-filestorage

mkdir build && cd build
cmake ..
make -j$(nproc)

./bin/fileserver
```

Server will start on `http://localhost:8080`

### Docker Deployment

```bash
docker build -t fileserver:latest .

docker run -d -p 8080:8080 \
  -v $(pwd)/files:/app/files \
  -v $(pwd)/config:/app/config \
  --name fileserver \
  fileserver:latest
```

### Basic Usage

```bash
# Authenticate
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username": "admin", "password": "admin123"}'

# Upload file
curl -X POST http://localhost:8080/api/v1/files \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -F "file=@document.pdf"

# List files
curl http://localhost:8080/api/v1/files \
  -H "Authorization: Bearer YOUR_TOKEN"
```

---

## Documentation

| Document | Description |
|----------|-------------|
| [API Reference](docs/api_reference.md) | REST API endpoint documentation |
| [Architecture](docs/architecture.md) | System design and components |
| [Quick Start Guide](docs/quick_start_guide.md) | Setup and configuration |
| [Deployment Guide](docs/deployment_guide.md) | Production deployment |
| [Development Guide](docs/development.md) | Development workflow |
| [Swagger UI](docs/swagger_ui.html) | Interactive API documentation |

---

## Architecture

The application follows a layered architecture:

```
┌─────────────────────────────────────┐
│       HTTP Server (RESTful API)     │
├─────────────────────────────────────┤
│      Authentication & Security      │
├─────────────────────────────────────┤
│        Business Logic Layer         │
├─────────────────────────────────────┤
│          Storage Layer              │
└─────────────────────────────────────┘
```

### Components

- **FileServer** - Main application singleton, manages server lifecycle
- **FileManager** - Handles file operations using Strategy pattern
- **AuthManager** - Authentication, authorization, JWT token management
- **ConfigManager** - Configuration loading and management
- **MimeTypeDetector** - File type detection
- **HTTP Server** - REST API request processing

See [Architecture Documentation](docs/architecture.md) for details.

---

## API Overview

Base URL: `http://localhost:8080/api/v1`

### Authentication

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/auth/login` | Authenticate user, get JWT token |
| POST | `/auth/logout` | Invalidate token |
| GET | `/auth/me` | Get current user information |
| POST | `/auth/change-password` | Change user password |

### File Management

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/files` | List files |
| GET | `/files/:filename` | Download file |
| POST | `/files` | Upload file |
| DELETE | `/files/:filename` | Delete file |
| GET | `/files/:filename/info` | Get file metadata |

### Administration

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/admin/users` | List users |
| POST | `/admin/users` | Create user |
| DELETE | `/admin/users/:username` | Delete user |
| PUT | `/admin/users/:username/role` | Update user role |

### System

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/health` | Health check |
| GET | `/metrics` | Performance metrics |
| GET | `/version` | Server version |

See [API Reference](docs/api_reference.md) for complete documentation.

---

## Configuration

Configuration file format (`config/server.json`):

```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "max_threads": 8
  },
  "storage": {
    "root_path": "./files",
    "max_file_size": 104857600,
    "allowed_extensions": ["*"]
  },
  "auth": {
    "token_lifetime_hours": 24,
    "secret_key_path": "./config/secret.key"
  },
  "logging": {
    "level": "info",
    "file": "./logs/server.log"
  }
}
```

### Environment Variables

```bash
export SERVER_PORT=8080
export SERVER_HOST=0.0.0.0
export STORAGE_ROOT=./files
export MAX_FILE_SIZE=104857600
export JWT_SECRET_KEY=your-secret-key
export TOKEN_LIFETIME=24
```

---

## Testing

### Run Tests

```bash
cd build
ctest --output-on-failure
```

### Test Coverage

- Total Tests: 73
- Pass Rate: 100%
- Test Categories:
  - Basic Tests: 2
  - FileManager Tests: 9
  - ConfigManager Tests: 12
  - MimeTypeDetector Tests: 5
  - AuthManager Tests: 41
  - CI Integration Tests: 4

### Run Specific Tests

```bash
# FileManager tests only
./bin/fileserver_tests --gtest_filter=FileManagerTest.*

# Verbose output
./bin/fileserver_tests --gtest_verbose
```

---

## CI/CD

### GitHub Actions Workflows

- **CI Pipeline** (`.github/workflows/ci.yml`)
  - Build for Debug and Release configurations
  - Run test suite (73 tests)
  - Code quality checks (clang-format, cppcheck)
  - Docker build verification

- **PR Validation** (`.github/workflows/pr-check.yml`)
  - Automated pull request checks
  - Code review requirements
  - Test coverage verification

- **Release Automation** (`.github/workflows/release.yml`)
  - Version tagging
  - Release notes generation
  - Docker image publishing

### Pre-commit Hooks

```bash
# Install hooks
bash scripts/setup-precommit.sh

# Includes:
# - clang-format (code formatting)
# - cppcheck (static analysis)
# - detect-secrets (secret detection)
```

### Check CI Status

```bash
bash scripts/check-ci-status.sh dev
```

See [CI/CD Documentation](docs/ci-cd.md) for details.

---

## Docker

### Build Image

```bash
docker build -t fileserver:1.0.0 .
```

### Docker Compose

```yaml
version: '3.8'

services:
  fileserver:
    image: fileserver:1.0.0
    ports:
      - "8080:8080"
    volumes:
      - ./files:/app/files
      - ./config:/app/config
      - ./logs:/app/logs
    environment:
      - SERVER_PORT=8080
      - JWT_SECRET_KEY=${JWT_SECRET}
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/health"]
      interval: 30s
      timeout: 10s
      retries: 3
```

```bash
docker-compose up -d
```

See [Deployment Guide](docs/deployment_guide.md) for production setup.

---

## Development

### Build from Source

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake \
  libssl-dev libcurl4-openssl-dev

# Build
git clone https://github.com/Nice0Man/simple-filestorage.git
cd simple-filestorage
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Project Structure

```
simple-filestorage/
├── CMakeLists.txt           # Build configuration
├── Dockerfile               # Docker image
├── include/                 # Header files
│   ├── core/               # Core components
│   ├── security/           # Authentication
│   └── utils/              # Utilities
├── src/                    # Implementation
├── tests/                  # Test suites
├── docs/                   # Documentation
├── scripts/                # Build scripts
└── config/                 # Configuration
```

### Contributing

1. Fork the repository
2. Create feature branch: `git checkout -b feature/my-feature`
3. Write tests for changes
4. Ensure tests pass: `ctest --output-on-failure`
5. Format code: `clang-format -i src/**/*.cpp`
6. Commit: `git commit -m 'feat: add feature'`
7. Push: `git push origin feature/my-feature`
8. Create Pull Request

### Commit Convention

Follow [Conventional Commits](https://www.conventionalcommits.org/):

- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation
- `test:` Tests
- `refactor:` Code refactoring
- `perf:` Performance improvement
- `chore:` Build/tool changes

See [Development Guide](docs/development.md) for workflow details.

---

## Performance

Benchmark results on Intel i7-9700K, 16GB RAM, SSD:

| Operation | Throughput | Latency (p50) | Latency (p99) |
|-----------|-----------|---------------|---------------|
| File Upload (1MB) | 1000 req/s | 5ms | 15ms |
| File Download (1MB) | 1200 req/s | 3ms | 10ms |
| List Files | 5000 req/s | 1ms | 3ms |
| Authentication | 3000 req/s | 2ms | 5ms |

---

## Roadmap

### Version 1.1 (Q1 2025)

- OAuth2 authentication support
- File versioning
- File compression
- WebSocket support for real-time updates
- Caching layer

### Version 1.2 (Q2 2025)

- Cloud storage backends (S3, Azure Blob, GCS)
- File sharing with expirable links
- Thumbnail generation
- Full-text search
- Web UI dashboard

### Version 2.0 (Q4 2025)

- Distributed storage
- Microservices architecture
- Built-in CDN capabilities
- GraphQL API
- Advanced analytics

See [Roadmap](docs/roadmap.md) for complete plan.

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

---

## Links

- **Documentation**: [Full Docs](docs/)
- **Issues**: [GitHub Issues](https://github.com/Nice0Man/simple-filestorage/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Nice0Man/simple-filestorage/discussions)

---

## Acknowledgments

- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - HTTP server library
- [nlohmann/json](https://github.com/nlohmann/json) - JSON library
- [jwt-cpp](https://github.com/Thalhammer/jwt-cpp) - JWT library
- [Google Test](https://github.com/google/googletest) - Testing framework
