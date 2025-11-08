# Download Google Test if not found
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Source files required for testing
set(TEST_COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/file_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/config_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/mime_type_detector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/logger.cpp
)

# Add database and security sources only if PostgreSQL is found
if(LIBPQ_FOUND)
    list(APPEND TEST_COMMON_SOURCES
        ${CMAKE_SOURCE_DIR}/src/security/auth_manager.cpp
        ${CMAKE_SOURCE_DIR}/src/security/access_control.cpp
        ${CMAKE_SOURCE_DIR}/src/database/user_repository.cpp
        ${CMAKE_SOURCE_DIR}/src/database/connection_pool.cpp
    )
else()
    message(WARNING "PostgreSQL not found - database and security tests will be skipped")
endif()

# Test sources
set(TEST_SOURCES
    test_basic.cpp
    test_file_manager.cpp
    test_config_manager.cpp
    test_mime_type_detector.cpp
    test_ci_integration.cpp
)

# Add auth manager tests only if PostgreSQL is found (auth depends on database)
if(LIBPQ_FOUND)
    list(APPEND TEST_SOURCES test_auth_manager.cpp)
else()
    message(WARNING "Skipping test_auth_manager.cpp - requires PostgreSQL")
endif()

# Create test executable
add_executable(fileserver_tests 
    ${TEST_SOURCES}
    ${TEST_COMMON_SOURCES}
)

# Link libraries
target_link_libraries(fileserver_tests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    jwt-cpp::jwt-cpp
)

# Add PostgreSQL if found
if(LIBPQ_FOUND)
    target_link_libraries(fileserver_tests PRIVATE ${LIBPQ_LIBRARIES})
    target_include_directories(fileserver_tests PRIVATE ${LIBPQ_INCLUDE_DIRS})
    if(LIBPQ_CFLAGS_OTHER)
        target_compile_options(fileserver_tests PRIVATE ${LIBPQ_CFLAGS_OTHER})
    endif()
    target_compile_definitions(fileserver_tests PRIVATE HAVE_POSTGRESQL)
endif()

# Include directories for tests
target_include_directories(fileserver_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Compiler options
target_compile_options(fileserver_tests 
    PRIVATE 
    -Wno-unused-parameter
)

# Set test properties
set_target_properties(fileserver_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(fileserver_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES 
        LABELS "unit"
        TIMEOUT 30
)
