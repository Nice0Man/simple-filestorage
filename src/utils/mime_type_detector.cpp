#include "utils/mime_type_detector.h"
#include <algorithm>
#include <filesystem>
#include <vector>

namespace fileserver {
namespace utils {

// Static member definitions
std::unordered_map<std::string, std::string> MimeTypeDetector::mime_types_;
std::unordered_map<std::string, std::string> MimeTypeDetector::custom_mime_types_;

std::string MimeTypeDetector::getMimeType(const std::string& file_path) {
    std::string extension = getFileExtension(file_path);
    return getMimeTypeByExtension(extension);
}

std::string MimeTypeDetector::getMimeTypeByExtension(const std::string& extension) {
    if (mime_types_.empty()) {
        initializeDefaultMimeTypes();
    }
    
    std::string lower_ext = toLowerCase(extension);
    
    // Remove leading dot if present
    if (!lower_ext.empty() && lower_ext[0] == '.') {
        lower_ext = lower_ext.substr(1);
    }
    
    // Check custom mime types first
    auto custom_it = custom_mime_types_.find(lower_ext);
    if (custom_it != custom_mime_types_.end()) {
        return custom_it->second;
    }
    
    // Check default mime types
    auto it = mime_types_.find(lower_ext);
    if (it != mime_types_.end()) {
        return it->second;
    }
    
    return "application/octet-stream"; // Default binary type
}

bool MimeTypeDetector::isAllowedFileType(const std::string& file_path) {
    std::string mime_type = getMimeType(file_path);
    
    // Define allowed MIME types
    static const std::vector<std::string> allowed_types = {
        "text/plain",
        "text/html",
        "text/css",
        "text/javascript",
        "application/json",
        "application/xml",
        "application/pdf",
        "application/msword",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
        "application/vnd.ms-excel",
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
        "application/vnd.ms-powerpoint",
        "application/vnd.openxmlformats-officedocument.presentationml.presentation",
        "image/jpeg",
        "image/png",
        "image/gif",
        "image/webp",
        "image/svg+xml",
        "audio/mpeg",
        "audio/wav",
        "audio/ogg",
        "video/mp4",
        "video/avi",
        "video/quicktime",
        "application/zip",
        "application/x-rar-compressed",
        "application/x-tar",
        "application/gzip"
    };
    
    return std::find(allowed_types.begin(), allowed_types.end(), mime_type) != allowed_types.end();
}

void MimeTypeDetector::addMimeTypeMapping(const std::string& extension, const std::string& mime_type) {
    std::string lower_ext = toLowerCase(extension);
    if (!lower_ext.empty() && lower_ext[0] == '.') {
        lower_ext = lower_ext.substr(1);
    }
    custom_mime_types_[lower_ext] = mime_type;
}

void MimeTypeDetector::initializeDefaultMimeTypes() {
    mime_types_ = {
        // Text files
        {"txt", "text/plain"},
        {"html", "text/html"},
        {"htm", "text/html"},
        {"css", "text/css"},
        {"js", "application/javascript"},
        {"json", "application/json"},
        {"xml", "application/xml"},
        {"csv", "text/csv"},
        
        // Documents
        {"pdf", "application/pdf"},
        {"doc", "application/msword"},
        {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {"xls", "application/vnd.ms-excel"},
        {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        {"ppt", "application/vnd.ms-powerpoint"},
        {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
        {"odt", "application/vnd.oasis.opendocument.text"},
        {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
        {"odp", "application/vnd.oasis.opendocument.presentation"},
        
        // Images
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif", "image/gif"},
        {"bmp", "image/bmp"},
        {"webp", "image/webp"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},
        {"tiff", "image/tiff"},
        {"tif", "image/tiff"},
        
        // Audio
        {"mp3", "audio/mpeg"},
        {"wav", "audio/wav"},
        {"ogg", "audio/ogg"},
        {"flac", "audio/flac"},
        {"aac", "audio/aac"},
        {"m4a", "audio/mp4"},
        
        // Video
        {"mp4", "video/mp4"},
        {"avi", "video/x-msvideo"},
        {"mov", "video/quicktime"},
        {"wmv", "video/x-ms-wmv"},
        {"flv", "video/x-flv"},
        {"webm", "video/webm"},
        {"mkv", "video/x-matroska"},
        
        // Archives
        {"zip", "application/zip"},
        {"rar", "application/x-rar-compressed"},
        {"7z", "application/x-7z-compressed"},
        {"tar", "application/x-tar"},
        {"gz", "application/gzip"},
        {"bz2", "application/x-bzip2"},
        
        // Executables and binaries
        {"exe", "application/x-msdownload"},
        {"msi", "application/x-msi"},
        {"deb", "application/x-debian-package"},
        {"rpm", "application/x-rpm"},
        
        // Fonts
        {"ttf", "font/ttf"},
        {"otf", "font/otf"},
        {"woff", "font/woff"},
        {"woff2", "font/woff2"},
        
        // Other
        {"bin", "application/octet-stream"},
        {"iso", "application/x-iso9660-image"},
        {"dmg", "application/x-apple-diskimage"}
    };
}

std::string MimeTypeDetector::getFileExtension(const std::string& file_path) {
    try {
        std::filesystem::path path(file_path);
        std::string extension = path.extension().string();
        return extension;
    } catch (const std::exception&) {
        // Fallback to manual parsing
        size_t dot_pos = file_path.find_last_of('.');
        if (dot_pos != std::string::npos && dot_pos < file_path.length() - 1) {
            return file_path.substr(dot_pos);
        }
        return "";
    }
}

std::string MimeTypeDetector::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

} // namespace utils
} // namespace fileserver
