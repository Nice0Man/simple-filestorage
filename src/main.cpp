#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <chrono>
#include "core/file_server.h"
#include "utils/logger.h"

using namespace fileserver;

// Global flag for graceful shutdown
std::atomic<bool> shutdown_requested{false};

/**
 * @brief Signal handler for graceful shutdown
 * @param signal Signal number
 */
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nReceived shutdown signal. Stopping server gracefully...\n";
        shutdown_requested = true;
    }
}

/**
 * @brief Print usage information
 * @param program_name Program name
 */
void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  -c, --config <path>    Configuration file path (default: config.json)\n"
              << "  -h, --help            Show this help message\n"
              << "  -v, --version         Show version information\n";
}

/**
 * @brief Print version information
 */
void printVersion() {
    std::cout << "FileServer v1.0.0\n"
              << "A high-performance C++ file server with REST API\n"
              << "Built with modern C++17 and design patterns\n";
}

/**
 * @brief Main entry point
 * @param argc Argument count
 * @param argv Argument values
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    // Read config path from environment variable or use default
    const char* env_config_path = std::getenv("CONFIG_PATH");
    std::string config_path = env_config_path ? env_config_path : "config.json";
    
    // Parse command line arguments (they override environment variables)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        }
        else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_path = argv[++i];
            } else {
                std::cerr << "Error: --config requires a path argument\n";
                printUsage(argv[0]);
                return 1;
            }
        }
        else {
            std::cerr << "Error: Unknown argument '" << arg << "'\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        // Initialize logger
        auto& logger = utils::Logger::getInstance();
        if (!logger.initialize("fileserver.log", utils::LogLevel::INFO)) {
            std::cerr << "Failed to initialize logger\n";
            return 1;
        }
        
        logger.info("Starting FileServer v1.0.0");
        
        // Get server instance and initialize
        auto& server = core::FileServer::getInstance();
        if (!server.initialize(config_path)) {
            logger.error("Failed to initialize server with config: " + config_path);
            std::cerr << "Failed to initialize server. Check configuration file: " << config_path << "\n";
            return 1;
        }
        
        // Start server
        if (!server.start()) {
            logger.error("Failed to start server");
            std::cerr << "Failed to start server\n";
            return 1;
        }
        
        logger.info("Server started successfully");
        std::cout << "FileServer started successfully. Press Ctrl+C to stop.\n";
        
        // Main loop - wait for shutdown signal
        while (!shutdown_requested && server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Graceful shutdown
        logger.info("Shutting down server...");
        server.stop();
        logger.info("Server stopped successfully");
        
        std::cout << "Server stopped successfully.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred\n";
        return 1;
    }
    
    return 0;
}
