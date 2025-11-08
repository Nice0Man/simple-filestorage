#pragma once

#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include "core/request_handler.h"
#include "utils/config_manager.h"
#include "utils/logger.h"
#include "database/connection_pool.h"
#include "database/user_repository.h"

namespace fileserver {
namespace core {

/**
 * @brief Main file server class implementing Singleton pattern
 * 
 * This class manages the HTTP server lifecycle and coordinates
 * all file operations through a unified interface.
 */
class FileServer {
public:
    // Singleton pattern implementation
    static FileServer& getInstance();
    
    // Delete copy constructor and assignment operator
    FileServer(const FileServer&) = delete;
    FileServer& operator=(const FileServer&) = delete;
    
    ~FileServer() = default;
    
    /**
     * @brief Initialize the server with configuration
     * @param config_path Path to configuration file
     * @return true if initialization successful
     */
    bool initialize(const std::string& config_path);
    
    /**
     * @brief Start the server
     * @return true if server started successfully
     */
    bool start();
    
    /**
     * @brief Stop the server gracefully
     */
    void stop();
    
    /**
     * @brief Check if server is running
     * @return true if server is running
     */
    bool isRunning() const;
    
    /**
     * @brief Get server configuration
     * @return Reference to config manager
     */
    const utils::ConfigManager& getConfig() const;

private:
    FileServer() = default;
    
    void serverLoop();
    void setupRoutes();
    
    static std::unique_ptr<FileServer> instance_;
    static std::mutex instance_mutex_;
    
    std::unique_ptr<RequestHandler> request_handler_;
    std::unique_ptr<utils::ConfigManager> config_manager_;
    utils::Logger* logger_;
    
    // Database components
    std::shared_ptr<database::ConnectionPool> connection_pool_;
    std::shared_ptr<database::UserRepository> user_repository_;
    
    std::atomic<bool> running_{false};
    std::thread server_thread_;
    
    std::string host_;
    int port_;
};

} // namespace core
} // namespace fileserver
