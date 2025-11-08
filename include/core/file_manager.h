#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "utils/logger.h"

namespace fileserver {
namespace core {

/**
 * @brief File information structure
 */
struct FileInfo {
    std::string name;
    std::string path;
    std::uintmax_t size;
    std::filesystem::file_time_type last_modified;
    bool is_directory;
    std::string mime_type;
};

/**
 * @brief Abstract base class for file operations (Strategy pattern)
 */
class FileOperationStrategy {
public:
    virtual ~FileOperationStrategy() = default;
    virtual bool execute(const std::string& path, const std::string& data = "") = 0;
    virtual std::string getResult() const = 0;
};

/**
 * @brief Concrete strategy for file upload
 */
class UploadStrategy : public FileOperationStrategy {
public:
    bool execute(const std::string& path, const std::string& data = "") override;
    std::string getResult() const override;
    
private:
    std::string result_;
};

/**
 * @brief Concrete strategy for file download
 */
class DownloadStrategy : public FileOperationStrategy {
public:
    bool execute(const std::string& path, const std::string& data = "") override;
    std::string getResult() const override;
    
private:
    std::string result_;
};

/**
 * @brief Concrete strategy for file deletion
 */
class DeleteStrategy : public FileOperationStrategy {
public:
    bool execute(const std::string& path, const std::string& data = "") override;
    std::string getResult() const override;
    
private:
    std::string result_;
};

/**
 * @brief File manager class using Strategy pattern
 * 
 * Manages all file operations through pluggable strategies
 */
class FileManager {
public:
    explicit FileManager(const std::string& root_directory);
    ~FileManager() = default;
    
    /**
     * @brief Set operation strategy
     * @param strategy Unique pointer to strategy
     */
    void setStrategy(std::unique_ptr<FileOperationStrategy> strategy);
    
    /**
     * @brief Execute current strategy
     * @param path File path
     * @param data Optional data for operation
     * @return true if operation successful
     */
    bool executeOperation(const std::string& path, const std::string& data = "");
    
    /**
     * @brief Get result of last operation
     * @return Result string
     */
    std::string getOperationResult() const;
    
    /**
     * @brief List files in directory
     * @param directory_path Directory to list
     * @return Vector of file information
     */
    std::vector<FileInfo> listFiles(const std::string& directory_path = "") const;
    
    /**
     * @brief Check if file exists
     * @param file_path Path to file
     * @return true if file exists
     */
    bool fileExists(const std::string& file_path) const;
    
    /**
     * @brief Get file size
     * @param file_path Path to file
     * @return File size in bytes
     */
    std::uintmax_t getFileSize(const std::string& file_path) const;
    
    /**
     * @brief Validate file path for security
     * @param file_path Path to validate
     * @return true if path is safe
     */
    bool isPathSafe(const std::string& file_path) const;

private:
    std::string root_directory_;
    std::unique_ptr<FileOperationStrategy> current_strategy_;
    utils::Logger* logger_;
    
    std::string getFullPath(const std::string& relative_path) const;
    bool isWithinRoot(const std::string& full_path) const;
};

} // namespace core
} // namespace fileserver
