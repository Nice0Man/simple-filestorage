#!/bin/bash
# Valgrind Memory Leak Testing Script for FileServer
# Usage: ./scripts/valgrind_test.sh [test_executable]

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
VALGRIND_SUPP="${PROJECT_ROOT}/valgrind.supp"
VALGRIND_LOG="${BUILD_DIR}/valgrind_output.log"
VALGRIND_XML="${BUILD_DIR}/valgrind_output.xml"

echo -e "${BLUE}╔═══════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   FileServer Valgrind Memory Leak Testing    ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════╝${NC}"
echo

# Check if valgrind is installed
if ! command -v valgrind &> /dev/null; then
    echo -e "${RED}✗ Error: valgrind is not installed${NC}"
    echo "Install it with: sudo apt-get install valgrind"
    exit 1
fi

echo -e "${GREEN}✓ Valgrind found: $(valgrind --version)${NC}"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}⚠ Build directory not found. Creating...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Build with debug symbols
echo -e "${BLUE}Building project with debug symbols...${NC}"
cd "$BUILD_DIR"

cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -fno-omit-frame-pointer" \
      ..

make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Build successful${NC}"
echo

# Valgrind options
VALGRIND_OPTS=(
    "--leak-check=full"           # Detailed leak info
    "--show-leak-kinds=all"       # Show all leak types
    "--track-origins=yes"         # Track uninitialized values
    "--verbose"                   # Verbose output
    "--log-file=${VALGRIND_LOG}"  # Log file
    "--xml=yes"                   # XML output
    "--xml-file=${VALGRIND_XML}"  # XML file
    "--suppressions=${VALGRIND_SUPP}" # Suppression file
    "--gen-suppressions=all"      # Generate suppressions
    "--num-callers=30"            # Stack depth
    "--malloc-fill=0xAA"          # Fill allocated memory
    "--free-fill=0xDD"            # Fill freed memory
)

# Run tests
echo -e "${BLUE}Running Valgrind tests...${NC}"
echo

TEST_EXECUTABLES=()

if [ $# -gt 0 ]; then
    # Run specific test
    TEST_EXECUTABLES=("$@")
else
    # Find all test executables
    TEST_EXECUTABLES=($(find "$BUILD_DIR/tests" -type f -executable -name "test_*" 2>/dev/null))
fi

if [ ${#TEST_EXECUTABLES[@]} -eq 0 ]; then
    echo -e "${YELLOW}⚠ No test executables found${NC}"
    exit 0
fi

echo -e "${GREEN}Found ${#TEST_EXECUTABLES[@]} test executable(s)${NC}"
echo

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

for test_exe in "${TEST_EXECUTABLES[@]}"; do
    test_name=$(basename "$test_exe")
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Testing: ${test_name}${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    
    # Run valgrind
    valgrind "${VALGRIND_OPTS[@]}" "$test_exe" 2>&1 | tee "${BUILD_DIR}/valgrind_${test_name}.log"
    VALGRIND_EXIT_CODE=${PIPESTATUS[0]}
    
    # Check results
    if grep -q "ERROR SUMMARY: 0 errors" "${VALGRIND_LOG}"; then
        if grep -q "definitely lost: 0 bytes" "${VALGRIND_LOG}" && \
           grep -q "indirectly lost: 0 bytes" "${VALGRIND_LOG}"; then
            echo -e "${GREEN}✓ ${test_name}: PASSED (No leaks detected)${NC}"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo -e "${YELLOW}⚠ ${test_name}: WARNINGS (Possible leaks)${NC}"
            grep "lost:" "${VALGRIND_LOG}" || true
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    else
        echo -e "${RED}✗ ${test_name}: FAILED (Memory errors detected)${NC}"
        grep "ERROR SUMMARY:" "${VALGRIND_LOG}" || true
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    echo
done

# Summary
echo -e "${BLUE}╔═══════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║              Test Summary                     ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════╝${NC}"
echo -e "Total tests:  ${TOTAL_TESTS}"
echo -e "${GREEN}Passed:       ${PASSED_TESTS}${NC}"
if [ $FAILED_TESTS -gt 0 ]; then
    echo -e "${RED}Failed:       ${FAILED_TESTS}${NC}"
else
    echo -e "Failed:       ${FAILED_TESTS}"
fi
echo
echo -e "${BLUE}Detailed logs:${NC}"
echo "  Text log: ${VALGRIND_LOG}"
echo "  XML log:  ${VALGRIND_XML}"
echo

# Test main application
echo -e "${BLUE}╔═══════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║      Testing Main Application (5 seconds)    ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════╝${NC}"
echo

MAIN_EXECUTABLE="${BUILD_DIR}/fileserver"
if [ -f "$MAIN_EXECUTABLE" ]; then
    echo -e "${BLUE}Starting fileserver with valgrind...${NC}"
    
    # Create temporary config
    TEMP_CONFIG=$(mktemp)
    cat > "$TEMP_CONFIG" << EOF
{
  "server": {
    "host": "0.0.0.0",
    "port": 8081,
    "root_directory": "/tmp/fileserver_test",
    "max_connections": 10,
    "timeout_seconds": 5
  },
  "database": {
    "enabled": false
  },
  "logging": {
    "level": "debug",
    "file": "/tmp/fileserver_test.log"
  }
}
EOF
    
    mkdir -p /tmp/fileserver_test
    
    # Run with timeout
    timeout 5s valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --log-file="${BUILD_DIR}/valgrind_fileserver.log" \
        --suppressions="${VALGRIND_SUPP}" \
        "$MAIN_EXECUTABLE" "$TEMP_CONFIG" 2>&1 &
    
    VALGRIND_PID=$!
    sleep 5
    
    # Kill gracefully
    kill -TERM $VALGRIND_PID 2>/dev/null || true
    wait $VALGRIND_PID 2>/dev/null || true
    
    # Check results
    if [ -f "${BUILD_DIR}/valgrind_fileserver.log" ]; then
        echo -e "${BLUE}Main application results:${NC}"
        grep "LEAK SUMMARY:" -A 5 "${BUILD_DIR}/valgrind_fileserver.log" || true
        grep "ERROR SUMMARY:" "${BUILD_DIR}/valgrind_fileserver.log" || true
    fi
    
    # Cleanup
    rm -f "$TEMP_CONFIG"
    rm -rf /tmp/fileserver_test
    
    echo -e "${GREEN}✓ Main application test completed${NC}"
else
    echo -e "${YELLOW}⚠ Main executable not found: ${MAIN_EXECUTABLE}${NC}"
fi

echo
echo -e "${BLUE}════════════════════════════════════════════════${NC}"

# Exit with appropriate code
if [ $FAILED_TESTS -gt 0 ]; then
    echo -e "${RED}Some tests failed. Check logs for details.${NC}"
    exit 1
else
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
fi

