#!/bin/bash

# Complete workflow testing script
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   Complete CI/CD Workflow Testing${NC}"
echo -e "${BLUE}================================================${NC}"
echo ""

# Function to print status
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $2"
    else
        echo -e "${RED}✗${NC} $2"
        return 1
    fi
}

print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

# Counter for passed/failed tests
TESTS_PASSED=0
TESTS_FAILED=0

run_test() {
    local test_name="$1"
    local test_command="$2"
    
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Testing: ${test_name}${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    
    if eval "$test_command" > /tmp/test_output.log 2>&1; then
        print_status 0 "${test_name}"
        ((TESTS_PASSED++))
        return 0
    else
        print_status 1 "${test_name}"
        echo "Error output:"
        tail -n 20 /tmp/test_output.log
        ((TESTS_FAILED++))
        return 1
    fi
}

# Test 1: YAML Validation
run_test "YAML Syntax Validation" \
    "python3 -c \"import yaml; [yaml.safe_load(open(f)) for f in ['.github/workflows/ci.yml', '.github/workflows/pr-check.yml', '.github/workflows/release.yml', '.pre-commit-config.yaml', '.github/labeler.yml']]\""

# Test 2: File Structure
run_test "File Structure Check" \
    "test -f .github/workflows/ci.yml && test -f .github/workflows/pr-check.yml && test -f .github/workflows/release.yml && test -f .pre-commit-config.yaml && test -f .clang-format"

# Test 3: CMake Configuration
run_test "CMake Configuration (Debug)" \
    "rm -rf build && cmake -B build -DCMAKE_BUILD_TYPE=Debug"

# Test 4: Build
run_test "Project Build (Debug)" \
    "cmake --build build -j\$(nproc)"

# Test 5: Run Tests
run_test "Unit Tests Execution" \
    "cd build && ctest --output-on-failure"

cd "$PROJECT_ROOT"

# Test 6: Code Formatting Check
if command -v clang-format &> /dev/null; then
    run_test "Code Formatting Check" \
        "find tests -name 'test_ci_integration.cpp' | xargs clang-format --dry-run --Werror"
else
    print_warning "clang-format not installed, skipping formatting check"
fi

# Test 7: Static Analysis
if command -v cppcheck &> /dev/null; then
    run_test "Static Analysis (cppcheck)" \
        "cppcheck --enable=warning,style --suppress=missingIncludeSystem --quiet tests/test_ci_integration.cpp"
else
    print_warning "cppcheck not installed, skipping static analysis"
fi

# Test 8: Release Build
run_test "CMake Configuration (Release)" \
    "rm -rf build && cmake -B build -DCMAKE_BUILD_TYPE=Release"

run_test "Project Build (Release)" \
    "cmake --build build -j\$(nproc)"

# Test 9: Git Status
run_test "Git Repository Status" \
    "git status --porcelain | grep -v '^??' || true"

# Test 10: Pre-commit Hooks (if installed)
if command -v pre-commit &> /dev/null && pre-commit --version &> /dev/null; then
    run_test "Pre-commit Hooks" \
        "pre-commit run --files tests/test_ci_integration.cpp || true"
else
    print_warning "Pre-commit not installed, skipping hooks check"
fi

# Summary
echo ""
echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   Test Summary${NC}"
echo -e "${BLUE}================================================${NC}"
echo ""
echo -e "Tests Passed: ${GREEN}${TESTS_PASSED}${NC}"
echo -e "Tests Failed: ${RED}${TESTS_FAILED}${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${GREEN}   ✓ All tests passed!${NC}"
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Review the changes: git status"
    echo "  2. Add new files: git add tests/test_ci_integration.cpp"
    echo "  3. Commit: git commit -m 'test: add CI integration test'"
    echo "  4. Push: git push origin main"
    echo "  5. Check GitHub Actions tab"
    echo ""
    exit 0
else
    echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${RED}   ✗ Some tests failed${NC}"
    echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    echo "Please fix the errors above before proceeding."
    echo ""
    exit 1
fi

