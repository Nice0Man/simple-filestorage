# Source files
set(SOURCES
    main.cpp
    core/file_server.cpp
    core/file_manager.cpp
    core/request_handler.cpp
    utils/logger.cpp
    utils/config_manager.cpp
    utils/mime_type_detector.cpp
    security/auth_manager.cpp
    security/access_control.cpp
    database/connection_pool.cpp
    database/user_repository.cpp
)

# Create executable
add_executable(fileserver ${SOURCES})

# Link libraries
target_link_libraries(fileserver 
    PRIVATE 
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    httplib::httplib
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    jwt-cpp::jwt-cpp
)

# Add PostgreSQL if found
if(LIBPQ_FOUND)
    target_link_libraries(fileserver PRIVATE ${LIBPQ_LIBRARIES})
    target_include_directories(fileserver PRIVATE ${LIBPQ_INCLUDE_DIRS})
    if(LIBPQ_CFLAGS_OTHER)
        target_compile_options(fileserver PRIVATE ${LIBPQ_CFLAGS_OTHER})
    endif()
    target_compile_definitions(fileserver PRIVATE HAVE_POSTGRESQL)
endif()

# Set target properties
set_target_properties(fileserver PROPERTIES
    OUTPUT_NAME "fileserver"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
