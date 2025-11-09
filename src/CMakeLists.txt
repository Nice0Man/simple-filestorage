# ================================
# Library: Utils (no dependencies)
# ================================
add_library(fileserver_utils STATIC
    utils/logger.cpp
    utils/config_manager.cpp
    utils/mime_type_detector.cpp
)

target_link_libraries(fileserver_utils
    PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

target_include_directories(fileserver_utils
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# ================================
# Library: Database
# ================================
add_library(fileserver_database STATIC
    database/connection_pool.cpp
    database/user_repository.cpp
)

target_link_libraries(fileserver_database
    PUBLIC
    fileserver_utils
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(fileserver_database
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Add PostgreSQL
if(LIBPQ_FOUND)
    target_link_libraries(fileserver_database PUBLIC ${LIBPQ_LIBRARIES})
    target_include_directories(fileserver_database PUBLIC ${LIBPQ_INCLUDE_DIRS})
    if(LIBPQ_CFLAGS_OTHER)
        target_compile_options(fileserver_database PUBLIC ${LIBPQ_CFLAGS_OTHER})
    endif()
    target_compile_definitions(fileserver_database PUBLIC HAVE_POSTGRESQL)
endif()

# ================================
# Library: Security
# ================================
add_library(fileserver_security STATIC
    security/auth_manager.cpp
    security/access_control.cpp
)

target_link_libraries(fileserver_security
    PUBLIC
    fileserver_database
    fileserver_utils
    jwt-cpp::jwt-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(fileserver_security
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# ================================
# Library: Core
# ================================
add_library(fileserver_core STATIC
    core/file_server.cpp
    core/file_manager.cpp
    core/request_handler.cpp
)

target_link_libraries(fileserver_core
    PUBLIC
    fileserver_security
    fileserver_database
    fileserver_utils
    httplib::httplib
    Threads::Threads
)

target_include_directories(fileserver_core
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# ================================
# Executable: fileserver
# ================================
add_executable(fileserver main.cpp)

# Link all libraries
target_link_libraries(fileserver 
    PRIVATE 
    fileserver_core
    fileserver_security
    fileserver_database
    fileserver_utils
)

# Set target properties
set_target_properties(fileserver PROPERTIES
    OUTPUT_NAME "fileserver"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Set library output directories
set_target_properties(
    fileserver_utils
    fileserver_database
    fileserver_security
    fileserver_core
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
