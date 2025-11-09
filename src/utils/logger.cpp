#include "utils/logger.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>

namespace fileserver {
namespace utils {

// Static member definitions
std::unique_ptr<Logger> Logger::instance_ = nullptr;
std::mutex Logger::instance_mutex_;

Logger& Logger::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (instance_ == nullptr) {
        instance_ = std::unique_ptr<Logger>(new Logger());
    }
    return *instance_;
}

Logger::~Logger() {
    close();
}

bool Logger::initialize(const std::string& log_file_path, LogLevel min_level) {
    try {
        // Create sinks
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file_path, 1024 * 1024 * 50, 5); // 50MB, 5 files
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");

        // Create logger with both sinks
        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        auto logger = std::make_shared<spdlog::logger>("fileserver", sinks.begin(), sinks.end());
        
        // Set level
        spdlog::level::level_enum spdlog_level;
        switch (min_level) {
            case LogLevel::DEBUG: spdlog_level = spdlog::level::debug; break;
            case LogLevel::INFO: spdlog_level = spdlog::level::info; break;
            case LogLevel::WARNING: spdlog_level = spdlog::level::warn; break;
            case LogLevel::ERROR: spdlog_level = spdlog::level::err; break;
            case LogLevel::CRITICAL: spdlog_level = spdlog::level::critical; break;
            default: spdlog_level = spdlog::level::info; break;
        }
        
        logger->set_level(spdlog_level);
        logger->flush_on(spdlog::level::warn);
        
        // Register logger
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
        
        min_level_ = min_level;
        initialized_ = true;
        
        info("Logger initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        return false;
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!initialized_) {
        return;
    }
    
    try {
        switch (level) {
            case LogLevel::DEBUG:
                spdlog::debug(message);
                break;
            case LogLevel::INFO:
                spdlog::info(message);
                break;
            case LogLevel::WARNING:
                spdlog::warn(message);
                break;
            case LogLevel::ERROR:
                spdlog::error(message);
                break;
            case LogLevel::CRITICAL:
                spdlog::critical(message);
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Logging error: " << e.what() << std::endl;
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::setMinLevel(LogLevel level) {
    min_level_ = level;
    if (initialized_) {
        spdlog::level::level_enum spdlog_level;
        switch (level) {
            case LogLevel::DEBUG: spdlog_level = spdlog::level::debug; break;
            case LogLevel::INFO: spdlog_level = spdlog::level::info; break;
            case LogLevel::WARNING: spdlog_level = spdlog::level::warn; break;
            case LogLevel::ERROR: spdlog_level = spdlog::level::err; break;
            case LogLevel::CRITICAL: spdlog_level = spdlog::level::critical; break;
            default: spdlog_level = spdlog::level::info; break;
        }
        spdlog::set_level(spdlog_level);
    }
}

void Logger::close() {
    if (initialized_) {
        // Simply mark as uninitialized
        // Don't interact with spdlog during shutdown to avoid use-after-free
        // spdlog will clean up its own resources via its destructors
        initialized_ = false;
    }
}

} // namespace utils
} // namespace fileserver
