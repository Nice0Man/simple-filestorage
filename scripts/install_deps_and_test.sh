#!/bin/bash
# Install dependencies and run tests with valgrind

set -e

echo "=== Installing dependencies ==="
sudo apt-get update
sudo apt-get install -y libpq-dev pkg-config valgrind g++ cmake make

echo ""
echo "=== Building project ==="
cd "$(dirname "$0")/.."
rm -rf build
mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -fno-omit-frame-pointer" \
      ..

make -j$(nproc)

echo ""
echo "=== Running tests ==="
ctest --output-on-failure

echo ""
echo "=== Running Valgrind memory check ==="
cd ..
./scripts/valgrind_test.sh

echo ""
echo "✓ All tests completed!"

