#include "core/file_manager.h"
#include "utils/mime_type_detector.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fileserver {
namespace core {

// UploadStrategy implementation
bool UploadStrategy::execute(const std::string& path, const std::string& data) {
    try {
        // Create directories if they don't exist
        std::filesystem::path file_path(path);
        std::filesystem::create_directories(file_path.parent_path());
        
        // Write file
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            result_ = "Failed to create file: " + path;
            return false;
        }
        
        file.write(data.c_str(), data.size());
        file.close();
        
        if (file.fail()) {
            result_ = "Failed to write file: " + path;
            return false;
        }
        
        result_ = "File uploaded successfully: " + path;
        return true;
        
    } catch (const std::exception& e) {
        result_ = "Upload error: " + std::string(e.what());
        return false;
    }
}

std::string UploadStrategy::getResult() const {
    return result_;
}

// DownloadStrategy implementation
bool DownloadStrategy::execute(const std::string& path, const std::string& data) {
    try {
        if (!std::filesystem::exists(path)) {
            result_ = "";
            return false;
        }
        
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            result_ = "";
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        result_ = buffer.str();
        
        return true;
        
    } catch (const std::exception& e) {
        result_ = "";
        return false;
    }
}

std::string DownloadStrategy::getResult() const {
    return result_;
}

// DeleteStrategy implementation
bool DeleteStrategy::execute(const std::string& path, const std::string& data) {
    try {
        if (!std::filesystem::exists(path)) {
            result_ = "File not found: " + path;
            return false;
        }
        
        std::error_code ec;
        bool removed = std::filesystem::remove_all(path, ec);
        
        if (!removed || ec) {
            result_ = "Failed to delete: " + path + " (" + ec.message() + ")";
            return false;
        }
        
        result_ = "File deleted successfully: " + path;
        return true;
        
    } catch (const std::exception& e) {
        result_ = "Delete error: " + std::string(e.what());
        return false;
    }
}

std::string DeleteStrategy::getResult() const {
    return result_;
}

// FileManager implementation
FileManager::FileManager(const std::string& root_directory) 
    : root_directory_(root_directory) {
    
    // Create root directory if it doesn't exist
    try {
        std::filesystem::create_directories(root_directory_);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create root directory: " << e.what() << std::endl;
    }
    
    // Get logger instance
    logger_ = &utils::Logger::getInstance();
}

void FileManager::setStrategy(std::unique_ptr<FileOperationStrategy> strategy) {
    current_strategy_ = std::move(strategy);
}

bool FileManager::executeOperation(const std::string& path, const std::string& data) {
    if (!current_strategy_) {
        return false;
    }
    
    std::string full_path = getFullPath(path);
    
    // Security check
    if (!isPathSafe(path)) {
        return false;
    }
    
    return current_strategy_->execute(full_path, data);
}

std::string FileManager::getOperationResult() const {
    if (!current_strategy_) {
        return "";
    }
    return current_strategy_->getResult();
}

std::vector<FileInfo> FileManager::listFiles(const std::string& directory_path) const {
    std::vector<FileInfo> files;
    
    try {
        std::string full_path = getFullPath(directory_path);
        
        if (!isPathSafe(directory_path) || !std::filesystem::exists(full_path)) {
            return files;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(full_path)) {
            FileInfo file_info;
            file_info.name = entry.path().filename().string();
            file_info.path = std::filesystem::relative(entry.path(), root_directory_).string();
            file_info.is_directory = entry.is_directory();
            
            if (!file_info.is_directory) {
                std::error_code ec;
                file_info.size = std::filesystem::file_size(entry.path(), ec);
                if (ec) {
                    file_info.size = 0;
                }
                
                file_info.mime_type = utils::MimeTypeDetector::getMimeType(file_info.name);
            } else {
                file_info.size = 0;
                file_info.mime_type = "inode/directory";
            }
            
            // Get last modified time
            std::error_code ec;
            file_info.last_modified = std::filesystem::last_write_time(entry.path(), ec);
            
            files.push_back(file_info);
        }
        
        // Sort files by name
        std::sort(files.begin(), files.end(), 
                 [](const FileInfo& a, const FileInfo& b) {
                     // Directories first, then files
                     if (a.is_directory != b.is_directory) {
                         return a.is_directory > b.is_directory;
                     }
                     return a.name < b.name;
                 });
        
    } catch (const std::exception& e) {
        if (logger_) {
            logger_->error("Error listing files: " + std::string(e.what()));
        }
    }
    
    return files;
}

bool FileManager::fileExists(const std::string& file_path) const {
    try {
        std::string full_path = getFullPath(file_path);
        return isPathSafe(file_path) && std::filesystem::exists(full_path);
    } catch (const std::exception&) {
        return false;
    }
}

std::uintmax_t FileManager::getFileSize(const std::string& file_path) const {
    try {
        std::string full_path = getFullPath(file_path);
        if (!isPathSafe(file_path) || !std::filesystem::exists(full_path)) {
            return 0;
        }
        
        std::error_code ec;
        auto size = std::filesystem::file_size(full_path, ec);
        return ec ? 0 : size;
        
    } catch (const std::exception&) {
        return 0;
    }
}

bool FileManager::isPathSafe(const std::string& file_path) const {
    try {
        std::string full_path = getFullPath(file_path);
        std::filesystem::path canonical_root = std::filesystem::canonical(root_directory_);
        std::filesystem::path canonical_path = std::filesystem::weakly_canonical(full_path);
        
        // If paths are equal, it's the root directory itself - allow it
        if (canonical_path == canonical_root) {
            return true;
        }
        
        // Check if the canonical path is within the root directory
        auto relative = std::filesystem::relative(canonical_path, canonical_root);
        return !relative.empty() && relative.native()[0] != '.';
        
    } catch (const std::exception&) {
        return false;
    }
}

std::string FileManager::getFullPath(const std::string& relative_path) const {
    if (relative_path.empty() || relative_path == "/") {
        return root_directory_;
    }
    
    std::filesystem::path full_path = std::filesystem::path(root_directory_) / relative_path;
    return full_path.string();
}

bool FileManager::isWithinRoot(const std::string& full_path) const {
    try {
        std::filesystem::path canonical_root = std::filesystem::canonical(root_directory_);
        std::filesystem::path canonical_path = std::filesystem::canonical(full_path);
        
        auto relative = std::filesystem::relative(canonical_path, canonical_root);
        return !relative.empty() && relative.native()[0] != '.';
        
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace core
} // namespace fileserver
