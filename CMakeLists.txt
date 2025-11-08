cmake_minimum_required(VERSION 3.16)
project(FileServer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Try to find PostgreSQL (optional for testing)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBPQ libpq)
    if(LIBPQ_FOUND)
        message(STATUS "PostgreSQL found: ${LIBPQ_VERSION}")
    else()
        message(WARNING "PostgreSQL not found via pkg-config, database features will be limited")
    endif()
else()
    message(WARNING "pkg-config not found, trying alternative PostgreSQL detection")
    # Try alternative PostgreSQL detection
    find_path(LIBPQ_INCLUDE_DIRS libpq-fe.h 
        PATHS /usr/include/postgresql /usr/local/include/postgresql)
    find_library(LIBPQ_LIBRARIES NAMES pq libpq
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
    
    if(LIBPQ_INCLUDE_DIRS AND LIBPQ_LIBRARIES)
        set(LIBPQ_FOUND TRUE)
        message(STATUS "PostgreSQL found (alternative detection)")
    else()
        set(LIBPQ_FOUND FALSE)
        message(WARNING "PostgreSQL not found, database features will be limited")
    endif()
endif()

# Include FetchContent for downloading dependencies
include(FetchContent)

# Download cpp-httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.3
)
FetchContent_MakeAvailable(httplib)

# Download nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Download spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# Download jwt-cpp
FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG v0.6.0
)
FetchContent_MakeAvailable(jwt-cpp)

# Add subdirectories
add_subdirectory(src)

# Enable testing
enable_testing()
add_subdirectory(tests)

# Installation
install(TARGETS fileserver
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
