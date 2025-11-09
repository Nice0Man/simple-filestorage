#!/bin/bash
# Quick incremental build script
# Only rebuilds changed files

set -e

cd "$(dirname "$0")/.."

# Colors
BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}Quick Build Script${NC}"

# Check if build directory exists
if [ ! -d "build" ]; then
    echo -e "${YELLOW}First time build - this will take longer${NC}"
    mkdir -p build
    cd build
    
    # Use ninja if available (faster)
    if command -v ninja &> /dev/null; then
        echo -e "${GREEN}Configuring with Ninja build system${NC}"
        cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_CXX_FLAGS_RELEASE="-O2" \
              ..
        echo -e "${GREEN}Building with Ninja...${NC}"
        ninja -j$(nproc) fileserver
    else
        echo -e "${GREEN}Configuring with Make build system${NC}"
        cmake -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_CXX_FLAGS_RELEASE="-O2" \
              ..
        echo -e "${GREEN}Building with Make...${NC}"
        make -j$(nproc) fileserver
    fi
else
    cd build
    
    # Detect which generator is already in use
    if [ -f "build.ninja" ]; then
        echo -e "${GREEN}Using Ninja (incremental)${NC}"
        ninja -j$(nproc) fileserver
    elif [ -f "Makefile" ]; then
        echo -e "${GREEN}Using Make (incremental)${NC}"
        make -j$(nproc) fileserver
    else
        echo -e "${YELLOW}Build system not detected, reconfiguring...${NC}"
        if command -v ninja &> /dev/null; then
            cmake -G Ninja ..
            ninja -j$(nproc) fileserver
        else
            cmake ..
            make -j$(nproc) fileserver
        fi
    fi
fi

echo ""
echo -e "${GREEN}✓ Build completed!${NC}"
echo "Binary: $(pwd)/bin/fileserver"
ls -lh bin/fileserver

