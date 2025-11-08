#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <mutex>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace fileserver {
namespace utils {

/**
 * @brief Log levels enumeration
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

/**
 * @brief Thread-safe logger class implementing Singleton pattern
 */
class Logger {
public:
    static Logger& getInstance();
    
    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    ~Logger();
    
    /**
     * @brief Initialize logger with file path
     * @param log_file_path Path to log file
     * @param min_level Minimum log level to write
     * @return true if initialization successful
     */
    bool initialize(const std::string& log_file_path, LogLevel min_level = LogLevel::INFO);
    
    /**
     * @brief Log message with specified level
     * @param level Log level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);
    
    /**
     * @brief Log debug message
     * @param message Message to log
     */
    void debug(const std::string& message);
    
    /**
     * @brief Log info message
     * @param message Message to log
     */
    void info(const std::string& message);
    
    /**
     * @brief Log warning message
     * @param message Message to log
     */
    void warning(const std::string& message);
    
    /**
     * @brief Log error message
     * @param message Message to log
     */
    void error(const std::string& message);
    
    /**
     * @brief Log critical message
     * @param message Message to log
     */
    void critical(const std::string& message);
    
    /**
     * @brief Set minimum log level
     * @param level Minimum level
     */
    void setMinLevel(LogLevel level);
    
    /**
     * @brief Close logger
     */
    void close();

private:
    Logger() = default;
    
    std::string getCurrentTimestamp() const;
    std::string levelToString(LogLevel level) const;
    
    static std::unique_ptr<Logger> instance_;
    static std::mutex instance_mutex_;
    
    std::ofstream log_file_;
    LogLevel min_level_ = LogLevel::INFO;
    std::mutex log_mutex_;
    bool initialized_ = false;
};

} // namespace utils
} // namespace fileserver
