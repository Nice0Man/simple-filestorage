#include "core/request_handler.h"
#include "utils/mime_type_detector.h"
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <chrono>

namespace fileserver {
namespace core {

// HttpResponse implementation
void HttpResponse::setStatus(int code, const std::string& message) {
    status_code = code;
    status_message = message;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void HttpResponse::setContent(const std::string& content, const std::string& content_type) {
    body = content;
    setHeader("Content-Type", content_type);
    setHeader("Content-Length", std::to_string(content.size()));
}

// Command implementations
UploadCommand::UploadCommand(std::shared_ptr<FileManager> file_manager)
    : file_manager_(file_manager) {}

void UploadCommand::execute(const HttpRequest& request, HttpResponse& response) {
    try {
        // Extract file data from multipart form
        auto file_it = request.form_data.find("file");
        auto filename_it = request.form_data.find("filename");
        auto path_it = request.form_data.find("path");
        
        if (file_it == request.form_data.end()) {
            nlohmann::json json_response;
            json_response["success"] = false;
            json_response["error"] = "No file provided";
            json_response["message"] = "Please upload a file";
            
            response.setStatus(400, "Bad Request");
            response.setContent(json_response.dump(), "application/json");
            return;
        }
        
        std::string file_data = file_it->second;
        
        // Determine file path
        std::string file_path;
        if (path_it != request.form_data.end() && !path_it->second.empty()) {
            file_path = path_it->second;
        } else if (filename_it != request.form_data.end() && !filename_it->second.empty()) {
            // Use filename if provided
            file_path = filename_it->second;
        } else {
            // Generate default filename
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
            file_path = "file_" + std::to_string(timestamp) + ".dat";
        }
        
        // Set upload strategy
        file_manager_->setStrategy(std::make_unique<UploadStrategy>());
        
        if (file_manager_->executeOperation(file_path, file_data)) {
            nlohmann::json json_response;
            json_response["success"] = true;
            json_response["message"] = "File uploaded successfully";
            json_response["filename"] = file_path;
            json_response["size"] = file_data.size();
            
            response.setStatus(201, "Created");
            response.setContent(json_response.dump(), "application/json");
        } else {
            nlohmann::json json_response;
            json_response["success"] = false;
            json_response["error"] = file_manager_->getOperationResult();
            
            response.setStatus(500, "Internal Server Error");
            response.setContent(json_response.dump(), "application/json");
        }
        
    } catch (const std::exception& e) {
        nlohmann::json json_response;
        json_response["success"] = false;
        json_response["error"] = e.what();
        
        response.setStatus(500, "Internal Server Error");
        response.setContent(json_response.dump(), "application/json");
    }
}

DownloadCommand::DownloadCommand(std::shared_ptr<FileManager> file_manager)
    : file_manager_(file_manager) {}

void DownloadCommand::execute(const HttpRequest& request, HttpResponse& response) {
    try {
        // Extract file path from URL
        std::string file_path = request.path;
        if (file_path.find("/api/v1/files/download/") == 0) {
            file_path = file_path.substr(23); // Remove "/api/v1/files/download/"
        }
        
        if (!file_manager_->fileExists(file_path)) {
            response.setStatus(404, "Not Found");
            response.setContent("{\"error\":\"File not found\"}", "application/json");
            return;
        }
        
        // Set download strategy
        file_manager_->setStrategy(std::make_unique<DownloadStrategy>());
        
        if (file_manager_->executeOperation(file_path, "")) {
            std::string file_content = file_manager_->getOperationResult();
            std::string mime_type = utils::MimeTypeDetector::getMimeType(file_path);
            
            response.setStatus(200, "OK");
            response.setHeader("Content-Type", mime_type);
            response.setHeader("Content-Disposition", "attachment; filename=\"" + 
                             std::filesystem::path(file_path).filename().string() + "\"");
            response.body = file_content;
        } else {
            response.setStatus(500, "Internal Server Error");
            response.setContent("{\"error\":\"Failed to read file\"}", "application/json");
        }
        
    } catch (const std::exception& e) {
        nlohmann::json json_response;
        json_response["success"] = false;
        json_response["error"] = e.what();
        
        response.setStatus(500, "Internal Server Error");
        response.setContent(json_response.dump(), "application/json");
    }
}

DeleteCommand::DeleteCommand(std::shared_ptr<FileManager> file_manager)
    : file_manager_(file_manager) {}

void DeleteCommand::execute(const HttpRequest& request, HttpResponse& response) {
    try {
        // Extract file path from URL
        std::string file_path = request.path;
        if (file_path.find("/api/v1/files/") == 0) {
            file_path = file_path.substr(14); // Remove "/api/v1/files/"
        }
        
        if (!file_manager_->fileExists(file_path)) {
            response.setStatus(404, "Not Found");
            response.setContent("{\"error\":\"File not found\"}", "application/json");
            return;
        }
        
        // Set delete strategy
        file_manager_->setStrategy(std::make_unique<DeleteStrategy>());
        
        if (file_manager_->executeOperation(file_path, "")) {
            nlohmann::json json_response;
            json_response["success"] = true;
            json_response["message"] = file_manager_->getOperationResult();
            
            response.setStatus(200, "OK");
            response.setContent(json_response.dump(), "application/json");
        } else {
            nlohmann::json json_response;
            json_response["success"] = false;
            json_response["error"] = file_manager_->getOperationResult();
            
            response.setStatus(500, "Internal Server Error");
            response.setContent(json_response.dump(), "application/json");
        }
        
    } catch (const std::exception& e) {
        nlohmann::json json_response;
        json_response["success"] = false;
        json_response["error"] = e.what();
        
        response.setStatus(500, "Internal Server Error");
        response.setContent(json_response.dump(), "application/json");
    }
}

ListCommand::ListCommand(std::shared_ptr<FileManager> file_manager)
    : file_manager_(file_manager) {}

void ListCommand::execute(const HttpRequest& request, HttpResponse& response) {
    try {
        // Extract directory path from query parameters
        std::string dir_path = "";
        auto path_it = request.headers.find("path");
        if (path_it != request.headers.end()) {
            dir_path = path_it->second;
        }
        
        auto files = file_manager_->listFiles(dir_path);
        
        nlohmann::json json_response;
        json_response["success"] = true;
        json_response["data"]["files"] = nlohmann::json::array();
        
        for (const auto& file : files) {
            nlohmann::json file_json;
            file_json["name"] = file.name;
            file_json["path"] = file.path;
            file_json["size"] = file.size;
            file_json["mime_type"] = file.mime_type;
            file_json["is_directory"] = file.is_directory;
            
            // Convert file time to timestamp
            auto time_t = std::chrono::system_clock::to_time_t(
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    file.last_modified - std::filesystem::file_time_type::clock::now() + 
                    std::chrono::system_clock::now()));
            file_json["modified_at"] = time_t;
            
            json_response["data"]["files"].push_back(file_json);
        }
        
        json_response["data"]["count"] = files.size();
        
        response.setStatus(200, "OK");
        response.setContent(json_response.dump(), "application/json");
        
    } catch (const std::exception& e) {
        nlohmann::json json_response;
        json_response["success"] = false;
        json_response["error"] = e.what();
        
        response.setStatus(500, "Internal Server Error");
        response.setContent(json_response.dump(), "application/json");
    }
}

// RequestHandler implementation
RequestHandler::RequestHandler(std::shared_ptr<FileManager> file_manager, 
                               std::shared_ptr<database::UserRepository> user_repository)
    : file_manager_(file_manager), user_repository_(user_repository) {
    
    logger_ = &utils::Logger::getInstance();
    
    // Initialize auth manager with database support
    if (user_repository_) {
        auth_manager_ = std::make_shared<security::AuthManager>(user_repository_);
        logger_->info("AuthManager initialized with PostgreSQL database");
    } else {
        // Fallback to file-based authentication
        auth_manager_ = std::make_shared<security::AuthManager>();
        auth_manager_->initialize("/app/config/users.json");
        logger_->info("AuthManager initialized with file-based authentication");
    }
    
    setupDefaultRoutes();
}

void RequestHandler::handleRequest(const HttpRequest& request, HttpResponse& response) {
    try {
        // Set CORS headers
        response.setHeader("Access-Control-Allow-Origin", "*");
        response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        
        // Handle OPTIONS requests
        if (request.method == "OPTIONS") {
            response.setStatus(200, "OK");
            return;
        }
        
        // Check authentication for protected routes
        if (!isPublicEndpoint(request.path)) {
            if (!authenticate(request)) {
                response.setStatus(401, "Unauthorized");
                nlohmann::json error_response;
                error_response["error"] = "Authentication required";
                error_response["message"] = "Please provide a valid Bearer token in Authorization header";
                response.setContent(error_response.dump(), "application/json");
                return;
            }
        }
        
        // Find matching route
        std::string route_key = getRouteKey(request.method, request.path);
        auto route_it = routes_.find(route_key);
        
        if (route_it != routes_.end()) {
            route_it->second->execute(request, response);
        } else {
            handleNotFound(response);
        }
        
    } catch (const std::exception& e) {
        handleInternalError(response, e.what());
    }
}

void RequestHandler::registerCommand(const std::string& method, const std::string& path, 
                                   std::unique_ptr<Command> command) {
    std::string route_key = getRouteKey(method, path);
    routes_[route_key] = std::move(command);
}

void RequestHandler::setupDefaultRoutes() {
    // OAuth2 / Authentication endpoints
    registerCommand("POST", "/api/v1/auth/login", std::make_unique<LoginCommand>(auth_manager_));
    registerCommand("POST", "/api/v1/auth/register", std::make_unique<RegisterCommand>(auth_manager_));
    registerCommand("POST", "/api/v1/auth/refresh", std::make_unique<RefreshTokenCommand>(auth_manager_));
    registerCommand("POST", "/api/v1/auth/logout", std::make_unique<LogoutCommand>(auth_manager_));
    registerCommand("GET", "/api/v1/auth/me", std::make_unique<GetUserInfoCommand>(auth_manager_));
    
    // File operations
    registerCommand("POST", "/api/v1/files/upload", std::make_unique<UploadCommand>(file_manager_));
    registerCommand("GET", "/api/v1/files/download/*", std::make_unique<DownloadCommand>(file_manager_));
    registerCommand("DELETE", "/api/v1/files/*", std::make_unique<DeleteCommand>(file_manager_));
    registerCommand("GET", "/api/v1/files", std::make_unique<ListCommand>(file_manager_));
    
    // Health check
    registerCommand("GET", "/health", std::make_unique<HealthCommand>());
}

std::string RequestHandler::getRouteKey(const std::string& method, const std::string& path) const {
    // Simple route matching - in production, use a proper router
    if (path.find("/api/v1/files/download/") == 0) {
        return method + ":/api/v1/files/download/*";
    }
    if (path.find("/api/v1/files/") == 0 && method == "DELETE") {
        return method + ":/api/v1/files/*";
    }
    return method + ":" + path;
}

bool RequestHandler::authenticate(const HttpRequest& request) const {
    auto auth_it = request.headers.find("Authorization");
    if (auth_it == request.headers.end()) {
        return false;
    }
    
    std::string auth_header = auth_it->second;
    if (auth_header.find("Bearer ") != 0) {
        return false;
    }
    
    std::string token = auth_header.substr(7); // Remove "Bearer "
    return auth_manager_->validateToken(token);
}

void RequestHandler::handleNotFound(HttpResponse& response) const {
    nlohmann::json json_response;
    json_response["success"] = false;
    json_response["error"] = "Endpoint not found";
    
    response.setStatus(404, "Not Found");
    response.setContent(json_response.dump(), "application/json");
}

void RequestHandler::handleMethodNotAllowed(HttpResponse& response) const {
    nlohmann::json json_response;
    json_response["success"] = false;
    json_response["error"] = "Method not allowed";
    
    response.setStatus(405, "Method Not Allowed");
    response.setContent(json_response.dump(), "application/json");
}

void RequestHandler::handleInternalError(HttpResponse& response, const std::string& error) const {
    nlohmann::json json_response;
    json_response["success"] = false;
    json_response["error"] = "Internal server error";
    json_response["details"] = error;
    
    response.setStatus(500, "Internal Server Error");
    response.setContent(json_response.dump(), "application/json");
    
    if (logger_) {
        logger_->error("Internal error: " + error);
    }
}

bool RequestHandler::isPublicEndpoint(const std::string& path) const {
    // List of public endpoints that don't require authentication
    static const std::vector<std::string> public_endpoints = {
        "/health",
        "/api/v1/auth/login",
        "/api/v1/auth/register",
        "/docs",
        "/swagger.yaml"
    };
    
    for (const auto& endpoint : public_endpoints) {
        if (path == endpoint || path.find(endpoint) == 0) {
            return true;
        }
    }
    
    return false;
}

// HealthCommand implementation
void HealthCommand::execute(const HttpRequest& request, HttpResponse& response) {
    nlohmann::json json_response;
    json_response["status"] = "healthy";
    json_response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    json_response["version"] = "1.0.0";
    
    response.setStatus(200, "OK");
    response.setContent(json_response.dump(), "application/json");
}

// OAuth2 Command implementations

// LoginCommand
LoginCommand::LoginCommand(std::shared_ptr<security::AuthManager> auth_manager)
    : auth_manager_(auth_manager) {}

void LoginCommand::execute(const HttpRequest& request, HttpResponse& response) {
    try {
        // Parse request body
        auto json_body = nlohmann::json::parse(request.body);
        
        // Validate required fields
        if (!json_body.contains("username") || !json_body.contains("password")) {
            nlohmann::json error_response;
            error_response["error"] = "Invalid request";
            error_response["message"] = "Username and password are required";
            response.setStatus(400, "Bad Request");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        std::string username = json_body["username"];
        std::string password = json_body["password"];
        
        // Authenticate user
        std::string access_token = auth_manager_->authenticate(username, password);
        
        if (access_token.empty()) {
            nlohmann::json error_response;
            error_response["error"] = "Authentication failed";
            error_response["message"] = "Invalid username or password";
            response.setStatus(401, "Unauthorized");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        // Get user info
        auto user = auth_manager_->getUserByToken(access_token);
        
        // Success response (OAuth2 format)
        nlohmann::json success_response;
        success_response["access_token"] = access_token;
        success_response["token_type"] = "Bearer";
        success_response["expires_in"] = 3600; // 1 hour in seconds
        
        if (user) {
            success_response["user"] = {
                {"username", user->username},
                {"role", user->role}
            };
        }
        
        response.setStatus(200, "OK");
        response.setContent(success_response.dump(), "application/json");
        
    } catch (const nlohmann::json::exception& e) {
        nlohmann::json error_response;
        error_response["error"] = "Invalid JSON";
        error_response["message"] = e.what();
        response.setStatus(400, "Bad Request");
        response.setContent(error_response.dump(), "application/json");
    } catch (const std::exception& e) {
        nlohmann::json error_response;
        error_response["error"] = "Internal server error";
        error_response["message"] = e.what();
        response.setStatus(500, "Internal Server Error");
        response.setContent(error_response.dump(), "application/json");
    }
}

// RegisterCommand
RegisterCommand::RegisterCommand(std::shared_ptr<security::AuthManager> auth_manager)
    : auth_manager_(auth_manager) {}

void RegisterCommand::execute(const HttpRequest& request, HttpResponse& response) {
    try {
        // Parse request body
        auto json_body = nlohmann::json::parse(request.body);
        
        // Validate required fields
        if (!json_body.contains("username") || !json_body.contains("password")) {
            nlohmann::json error_response;
            error_response["error"] = "Invalid request";
            error_response["message"] = "Username and password are required";
            response.setStatus(400, "Bad Request");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        std::string username = json_body["username"];
        std::string password = json_body["password"];
        std::string role = json_body.value("role", "user"); // Default role is "user"
        
        // Validate username format
        if (username.length() < 3) {
            nlohmann::json error_response;
            error_response["error"] = "Invalid username";
            error_response["message"] = "Username must be at least 3 characters long";
            response.setStatus(400, "Bad Request");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        // Validate password strength
        if (password.length() < 6) {
            nlohmann::json error_response;
            error_response["error"] = "Weak password";
            error_response["message"] = "Password must be at least 6 characters long";
            response.setStatus(400, "Bad Request");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        // Register user
        bool success = auth_manager_->addUser(username, password, role);
        
        if (!success) {
            nlohmann::json error_response;
            error_response["error"] = "Registration failed";
            error_response["message"] = "Username already exists";
            response.setStatus(409, "Conflict");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        // Auto-login after registration
        std::string access_token = auth_manager_->authenticate(username, password);
        
        // Success response
        nlohmann::json success_response;
        success_response["message"] = "User registered successfully";
        success_response["access_token"] = access_token;
        success_response["token_type"] = "Bearer";
        success_response["expires_in"] = 3600;
        success_response["user"] = {
            {"username", username},
            {"role", role}
        };
        
        response.setStatus(201, "Created");
        response.setContent(success_response.dump(), "application/json");
        
    } catch (const nlohmann::json::exception& e) {
        nlohmann::json error_response;
        error_response["error"] = "Invalid JSON";
        error_response["message"] = e.what();
        response.setStatus(400, "Bad Request");
        response.setContent(error_response.dump(), "application/json");
    } catch (const std::exception& e) {
        nlohmann::json error_response;
        error_response["error"] = "Internal server error";
        error_response["message"] = e.what();
        response.setStatus(500, "Internal Server Error");
        response.setContent(error_response.dump(), "application/json");
    }
}

// RefreshTokenCommand
RefreshTokenCommand::RefreshTokenCommand(std::shared_ptr<security::AuthManager> auth_manager)
    : auth_manager_(auth_manager) {}

void RefreshTokenCommand::execute(const HttpRequest& request, HttpResponse& response) {
    try {
        // Get current token from Authorization header
        auto auth_it = request.headers.find("Authorization");
        if (auth_it == request.headers.end()) {
            nlohmann::json error_response;
            error_response["error"] = "No token provided";
            error_response["message"] = "Authorization header is missing";
            response.setStatus(401, "Unauthorized");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        std::string auth_header = auth_it->second;
        if (auth_header.find("Bearer ") != 0) {
            nlohmann::json error_response;
            error_response["error"] = "Invalid token format";
            error_response["message"] = "Use 'Bearer <token>' format";
            response.setStatus(401, "Unauthorized");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        std::string old_token = auth_header.substr(7);
        
        // Validate old token
        if (!auth_manager_->validateToken(old_token)) {
            nlohmann::json error_response;
            error_response["error"] = "Invalid token";
            error_response["message"] = "Token is expired or invalid";
            response.setStatus(401, "Unauthorized");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        // Get user from old token
        auto user = auth_manager_->getUserByToken(old_token);
        if (!user) {
            nlohmann::json error_response;
            error_response["error"] = "User not found";
            response.setStatus(404, "Not Found");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        // Revoke old token
        auth_manager_->revokeToken(old_token);
        
        // Generate new token
        std::string new_token = auth_manager_->authenticate(user->username, ""); // Internal refresh
        
        // In real implementation, you'd need a separate method for token refresh
        // For now, we simulate it by keeping the user logged in
        
        // Success response
        nlohmann::json success_response;
        success_response["access_token"] = new_token.empty() ? old_token : new_token;
        success_response["token_type"] = "Bearer";
        success_response["expires_in"] = 3600;
        
        response.setStatus(200, "OK");
        response.setContent(success_response.dump(), "application/json");
        
    } catch (const std::exception& e) {
        nlohmann::json error_response;
        error_response["error"] = "Internal server error";
        error_response["message"] = e.what();
        response.setStatus(500, "Internal Server Error");
        response.setContent(error_response.dump(), "application/json");
    }
}

// LogoutCommand
LogoutCommand::LogoutCommand(std::shared_ptr<security::AuthManager> auth_manager)
    : auth_manager_(auth_manager) {}

void LogoutCommand::execute(const HttpRequest& request, HttpResponse& response) {
    try {
        // Get token from Authorization header
        auto auth_it = request.headers.find("Authorization");
        if (auth_it == request.headers.end()) {
            nlohmann::json error_response;
            error_response["error"] = "No token provided";
            error_response["message"] = "Authorization header is missing";
            response.setStatus(401, "Unauthorized");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        std::string auth_header = auth_it->second;
        if (auth_header.find("Bearer ") != 0) {
            nlohmann::json error_response;
            error_response["error"] = "Invalid token format";
            response.setStatus(401, "Unauthorized");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        std::string token = auth_header.substr(7);
        
        // Revoke token
        bool success = auth_manager_->revokeToken(token);
        
        // Success response
        nlohmann::json success_response;
        success_response["message"] = success ? "Logged out successfully" : "Token already invalid";
        
        response.setStatus(200, "OK");
        response.setContent(success_response.dump(), "application/json");
        
    } catch (const std::exception& e) {
        nlohmann::json error_response;
        error_response["error"] = "Internal server error";
        error_response["message"] = e.what();
        response.setStatus(500, "Internal Server Error");
        response.setContent(error_response.dump(), "application/json");
    }
}

// GetUserInfoCommand
GetUserInfoCommand::GetUserInfoCommand(std::shared_ptr<security::AuthManager> auth_manager)
    : auth_manager_(auth_manager) {}

void GetUserInfoCommand::execute(const HttpRequest& request, HttpResponse& response) {
    try {
        // Get token from Authorization header
        auto auth_it = request.headers.find("Authorization");
        if (auth_it == request.headers.end()) {
            nlohmann::json error_response;
            error_response["error"] = "No token provided";
            response.setStatus(401, "Unauthorized");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        std::string auth_header = auth_it->second;
        if (auth_header.find("Bearer ") != 0) {
            nlohmann::json error_response;
            error_response["error"] = "Invalid token format";
            response.setStatus(401, "Unauthorized");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        std::string token = auth_header.substr(7);
        
        // Get user info
        auto user = auth_manager_->getUserByToken(token);
        
        if (!user) {
            nlohmann::json error_response;
            error_response["error"] = "User not found";
            error_response["message"] = "Invalid or expired token";
            response.setStatus(404, "Not Found");
            response.setContent(error_response.dump(), "application/json");
            return;
        }
        
        // Success response
        nlohmann::json success_response;
        success_response["username"] = user->username;
        success_response["role"] = user->role;
        success_response["is_active"] = user->is_active;
        
        // Convert time_point to timestamp
        auto created_time = std::chrono::system_clock::to_time_t(user->created_at);
        auto last_login_time = std::chrono::system_clock::to_time_t(user->last_login);
        
        success_response["created_at"] = created_time;
        success_response["last_login"] = last_login_time;
        
        response.setStatus(200, "OK");
        response.setContent(success_response.dump(), "application/json");
        
    } catch (const std::exception& e) {
        nlohmann::json error_response;
        error_response["error"] = "Internal server error";
        error_response["message"] = e.what();
        response.setStatus(500, "Internal Server Error");
        response.setContent(error_response.dump(), "application/json");
    }
}

} // namespace core
} // namespace fileserver
