# Development Guide

## Setting Up Development Environment

### Prerequisites

- C++17 compatible compiler (GCC 11+, Clang 14+, or MSVC 2019+)
- CMake 3.16+
- OpenSSL development libraries
- PostgreSQL client libraries (optional)
- Python 3.7+ (for pre-commit hooks)

### Initial Setup

1. Clone the repository:
```bash
git clone <repository-url>
cd simple-filestorage
```

2. Install pre-commit hooks:
```bash
# On Linux/macOS
bash scripts/setup-precommit.sh

# On Windows
powershell scripts/setup-precommit.ps1
```

## Pre-commit Hooks

Pre-commit hooks automatically run checks before each commit to ensure code quality.

### Installed Hooks

1. **General Checks:**
   - Trailing whitespace removal
   - End-of-file fixer
   - YAML validation
   - Large file detection (max 1MB)
   - Merge conflict detection

2. **C++ Specific:**
   - `clang-format`: Code formatting
   - `cppcheck`: Static analysis
   - Include guard validation
   - Debug print detection

3. **Build System:**
   - `cmake-format`: CMake file formatting
   - `cmake-lint`: CMake file linting

4. **Security:**
   - `detect-secrets`: Secret detection

### Manual Execution

Run all hooks on all files:
```bash
pre-commit run --all-files
```

Run specific hook:
```bash
pre-commit run clang-format --all-files
```

Update hooks to latest version:
```bash
pre-commit autoupdate
```

Skip hooks (not recommended):
```bash
git commit --no-verify
```

## Code Formatting

The project uses `clang-format` for consistent code formatting.

### Format Code Manually

Format a single file:
```bash
clang-format -i path/to/file.cpp
```

Format all C++ files:
```bash
find src include tests -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | \
  xargs clang-format -i
```

### Code Style Guidelines

- **Indentation:** 4 spaces (no tabs)
- **Line length:** 100 characters maximum
- **Braces:** Attached style (K&R)
- **Pointer/Reference:** Left aligned (`int* ptr`, not `int *ptr`)
- **Naming:**
  - Classes/Structs: `PascalCase`
  - Functions/Variables: `snake_case`
  - Constants: `UPPER_SNAKE_CASE`
  - Private members: `snake_case_` (trailing underscore)

## Building the Project

### Debug Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Release Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### With Specific Compiler

```bash
CC=gcc-12 CXX=g++-12 cmake -B build
cmake --build build
```

## Running Tests

### Run All Tests

```bash
cd build
ctest --output-on-failure
```

### Run Specific Test

```bash
cd build
./bin/fileserver_tests --gtest_filter=TestSuite.TestName
```

### Run with Verbose Output

```bash
cd build
ctest --verbose
```

## Continuous Integration

The project uses GitHub Actions for CI/CD.

### Workflows

1. **CI Workflow** (`.github/workflows/ci.yml`)
   - Runs on: Push to main/develop, Pull requests
   - Tests multiple OS versions and compilers
   - Includes code quality checks
   - Builds Docker image

### Workflow Jobs

#### Build and Test Matrix
- **Operating Systems:** Ubuntu 20.04, 22.04
- **Compilers:** GCC 11/12, Clang 14/15
- **Build Types:** Debug, Release

#### Code Quality
- clang-format validation
- cppcheck static analysis

#### Docker Build
- Validates Dockerfile
- Tests image creation

### Local CI Testing

Test the build matrix locally using Docker:

```bash
# Test with GCC
docker run --rm -v $(pwd):/workspace -w /workspace gcc:12 \
  bash -c "apt-get update && apt-get install -y cmake ninja-build libssl-dev && \
  cmake -B build -G Ninja && cmake --build build"

# Test with Clang
docker run --rm -v $(pwd):/workspace -w /workspace silkeh/clang:15 \
  bash -c "apt-get update && apt-get install -y cmake ninja-build libssl-dev && \
  cmake -B build -G Ninja && cmake --build build"
```

## Static Analysis

### cppcheck

Run static analysis:
```bash
cppcheck --enable=all \
  --suppress=missingIncludeSystem \
  --suppress=unusedFunction \
  -I include \
  src/
```

### clang-tidy

Run clang-tidy:
```bash
clang-tidy -p build src/*.cpp
```

## Debugging

### With GDB

```bash
cd build
gdb ./bin/fileserver
```

### With Valgrind

Check for memory leaks:
```bash
valgrind --leak-check=full --show-leak-kinds=all \
  ./build/bin/fileserver
```

## Common Issues

### Pre-commit Hook Failures

If pre-commit hooks fail:
1. Review the error messages
2. Fix the issues manually or let the hooks auto-fix them
3. Stage the changes: `git add .`
4. Commit again: `git commit`

### Formatting Issues

If clang-format check fails in CI:
```bash
# Format all files locally
pre-commit run clang-format --all-files

# Stage and commit the changes
git add .
git commit --amend --no-edit
```

### Build Failures

1. Clean build directory: `rm -rf build`
2. Update submodules: `git submodule update --init --recursive`
3. Check dependencies are installed
4. Try with a different compiler

## Contributing

1. Create a feature branch: `git checkout -b feature/my-feature`
2. Make your changes
3. Ensure all tests pass: `ctest`
4. Ensure code quality checks pass: `pre-commit run --all-files`
5. Commit with descriptive message (hooks will validate)
6. Push and create a pull request

### Commit Message Format

Follow conventional commits format:
```
type(scope): subject

body (optional)

footer (optional)
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

Examples:
```
feat(auth): add JWT token validation
fix(file-manager): resolve memory leak in upload handler
docs(readme): update build instructions
```

