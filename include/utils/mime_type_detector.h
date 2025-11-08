#pragma once

#include <string>
#include <unordered_map>

namespace fileserver {
namespace utils {

/**
 * @brief MIME type detector utility class
 */
class MimeTypeDetector {
public:
    /**
     * @brief Get MIME type by file extension
     * @param file_path Path to file
     * @return MIME type string
     */
    static std::string getMimeType(const std::string& file_path);
    
    /**
     * @brief Get MIME type by extension
     * @param extension File extension (with or without dot)
     * @return MIME type string
     */
    static std::string getMimeTypeByExtension(const std::string& extension);
    
    /**
     * @brief Check if file type is allowed for upload
     * @param file_path Path to file
     * @return true if file type is allowed
     */
    static bool isAllowedFileType(const std::string& file_path);
    
    /**
     * @brief Add custom MIME type mapping
     * @param extension File extension
     * @param mime_type MIME type
     */
    static void addMimeTypeMapping(const std::string& extension, const std::string& mime_type);

private:
    static std::unordered_map<std::string, std::string> mime_types_;
    static std::unordered_map<std::string, std::string> custom_mime_types_;
    
    static void initializeDefaultMimeTypes();
    static std::string getFileExtension(const std::string& file_path);
    static std::string toLowerCase(const std::string& str);
};

} // namespace utils
} // namespace fileserver
