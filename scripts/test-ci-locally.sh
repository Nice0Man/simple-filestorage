#!/bin/bash

# Script to test CI workflows locally
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "================================================"
echo "Testing CI Workflows Locally"
echo "================================================"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $2"
    else
        echo -e "${RED}✗${NC} $2"
    fi
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_info() {
    echo -e "${GREEN}ℹ${NC} $1"
}

# Check prerequisites
echo "Checking prerequisites..."
echo ""

# Check for required tools
check_tool() {
    if command -v $1 &> /dev/null; then
        print_status 0 "$1 is installed"
        return 0
    else
        print_status 1 "$1 is not installed"
        return 1
    fi
}

# Check basic tools
check_tool "cmake"
check_tool "g++"
check_tool "clang-format"
check_tool "cppcheck"

echo ""
echo "================================================"
echo "Step 1: Validate YAML Syntax"
echo "================================================"
echo ""

if command -v python3 &> /dev/null; then
    for file in .github/workflows/*.yml .pre-commit-config.yaml; do
        if python3 -c "import yaml; yaml.safe_load(open('$file'))" 2>/dev/null; then
            print_status 0 "$file"
        else
            print_status 1 "$file"
        fi
    done
else
    print_warning "Python3 not found, skipping YAML validation"
fi

echo ""
echo "================================================"
echo "Step 2: Code Formatting Check"
echo "================================================"
echo ""

if command -v clang-format &> /dev/null; then
    echo "Checking C++ code formatting..."
    UNFORMATTED=$(find src include tests -name '*.cpp' -o -name '*.h' -o -name '*.hpp' 2>/dev/null | \
        xargs clang-format --dry-run --Werror 2>&1 || true)
    
    if [ -z "$UNFORMATTED" ]; then
        print_status 0 "All files are properly formatted"
    else
        print_status 1 "Some files need formatting"
        echo "$UNFORMATTED"
        echo ""
        print_info "Run: find src include tests -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i"
    fi
else
    print_warning "clang-format not found, skipping format check"
fi

echo ""
echo "================================================"
echo "Step 3: Static Analysis (cppcheck)"
echo "================================================"
echo ""

if command -v cppcheck &> /dev/null; then
    echo "Running cppcheck..."
    if cppcheck --enable=warning,style,performance,portability \
        --suppress=missingIncludeSystem \
        --suppress=unusedFunction \
        --inline-suppr \
        --quiet \
        -I include \
        src/ 2>&1 | tee /tmp/cppcheck.log; then
        
        if [ ! -s /tmp/cppcheck.log ]; then
            print_status 0 "No issues found"
        else
            print_warning "Found some warnings (check output above)"
        fi
    else
        print_status 1 "cppcheck found errors"
    fi
else
    print_warning "cppcheck not found, skipping static analysis"
fi

echo ""
echo "================================================"
echo "Step 4: Build Test (Debug)"
echo "================================================"
echo ""

if [ -d "build" ]; then
    print_warning "Build directory exists, cleaning..."
    rm -rf build
fi

echo "Configuring CMake (Debug)..."
if cmake -B build -DCMAKE_BUILD_TYPE=Debug &> /tmp/cmake-config.log; then
    print_status 0 "CMake configuration successful"
    
    echo "Building project..."
    if cmake --build build -j$(nproc) &> /tmp/cmake-build.log; then
        print_status 0 "Build successful"
    else
        print_status 1 "Build failed"
        echo ""
        echo "Last 20 lines of build log:"
        tail -n 20 /tmp/cmake-build.log
    fi
else
    print_status 1 "CMake configuration failed"
    echo ""
    echo "Configuration log:"
    cat /tmp/cmake-config.log
fi

echo ""
echo "================================================"
echo "Step 5: Run Tests"
echo "================================================"
echo ""

if [ -d "build" ] && [ -f "build/bin/fileserver_tests" ]; then
    echo "Running unit tests..."
    cd build
    if ctest --output-on-failure --verbose; then
        print_status 0 "All tests passed"
    else
        print_status 1 "Some tests failed"
    fi
    cd ..
else
    print_warning "Test binary not found, skipping tests"
fi

echo ""
echo "================================================"
echo "Step 6: Build Test (Release)"
echo "================================================"
echo ""

rm -rf build

echo "Configuring CMake (Release)..."
if cmake -B build -DCMAKE_BUILD_TYPE=Release &> /tmp/cmake-config-release.log; then
    print_status 0 "CMake configuration successful"
    
    echo "Building project..."
    if cmake --build build -j$(nproc) &> /tmp/cmake-build-release.log; then
        print_status 0 "Release build successful"
    else
        print_status 1 "Release build failed"
    fi
else
    print_status 1 "CMake configuration failed"
fi

echo ""
echo "================================================"
echo "Step 7: Docker Build Test (optional)"
echo "================================================"
echo ""

if command -v docker &> /dev/null; then
    if [ -f "Dockerfile" ]; then
        echo "Building Docker image..."
        if docker build -t fileserver:test . &> /tmp/docker-build.log; then
            print_status 0 "Docker build successful"
            
            echo "Testing Docker image..."
            if docker run --rm fileserver:test --version 2>/dev/null || \
               docker run --rm fileserver:test --help 2>/dev/null || \
               docker run --rm fileserver:test 2>&1 | grep -q "fileserver"; then
                print_status 0 "Docker image runs"
            else
                print_warning "Docker image built but may have runtime issues"
            fi
        else
            print_status 1 "Docker build failed"
            echo ""
            echo "Last 20 lines of Docker build log:"
            tail -n 20 /tmp/docker-build.log
        fi
    else
        print_warning "Dockerfile not found, skipping Docker build"
    fi
else
    print_warning "Docker not found, skipping Docker build test"
fi

echo ""
echo "================================================"
echo "Summary"
echo "================================================"
echo ""
print_info "Local CI checks completed!"
echo ""
echo "Next steps:"
echo "  1. Fix any issues found above"
echo "  2. Commit your changes: git add . && git commit"
echo "  3. Push to GitHub: git push"
echo "  4. Check GitHub Actions tab for CI results"
echo ""
print_info "To install actionlint for workflow validation:"
echo "  bash <(curl https://raw.githubusercontent.com/rhysd/actionlint/main/scripts/download-actionlint.bash)"
echo ""
print_info "To run workflows locally with act:"
echo "  Install: https://github.com/nektos/act"
echo "  Run: act -l  # list workflows"
echo "  Run: act push  # simulate push event"
echo ""

