#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "core/file_manager.h"
#include "security/auth_manager.h"
#include "utils/logger.h"
#include "database/user_repository.h"

namespace fileserver {
namespace core {

/**
 * @brief HTTP request structure
 */
struct HttpRequest {
    std::string method;
    std::string path;
    std::string query_string;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::unordered_map<std::string, std::string> form_data;
};

/**
 * @brief HTTP response structure
 */
struct HttpResponse {
    int status_code = 200;
    std::string status_message = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    
    void setStatus(int code, const std::string& message);
    void setHeader(const std::string& key, const std::string& value);
    void setContent(const std::string& content, const std::string& content_type);
};

/**
 * @brief Abstract command interface (Command pattern)
 */
class Command {
public:
    virtual ~Command() = default;
    virtual void execute(const HttpRequest& request, HttpResponse& response) = 0;
};

/**
 * @brief Upload command implementation
 */
class UploadCommand : public Command {
public:
    explicit UploadCommand(std::shared_ptr<FileManager> file_manager);
    void execute(const HttpRequest& request, HttpResponse& response) override;
    
private:
    std::shared_ptr<FileManager> file_manager_;
};

/**
 * @brief Download command implementation
 */
class DownloadCommand : public Command {
public:
    explicit DownloadCommand(std::shared_ptr<FileManager> file_manager);
    void execute(const HttpRequest& request, HttpResponse& response) override;
    
private:
    std::shared_ptr<FileManager> file_manager_;
};

/**
 * @brief Delete command implementation
 */
class DeleteCommand : public Command {
public:
    explicit DeleteCommand(std::shared_ptr<FileManager> file_manager);
    void execute(const HttpRequest& request, HttpResponse& response) override;
    
private:
    std::shared_ptr<FileManager> file_manager_;
};

/**
 * @brief List files command implementation
 */
class ListCommand : public Command {
public:
    explicit ListCommand(std::shared_ptr<FileManager> file_manager);
    void execute(const HttpRequest& request, HttpResponse& response) override;
    
private:
    std::shared_ptr<FileManager> file_manager_;
};

/**
 * @brief Health check command implementation
 */
class HealthCommand : public Command {
public:
    void execute(const HttpRequest& request, HttpResponse& response) override;
};

/**
 * @brief OAuth2 Login command implementation
 */
class LoginCommand : public Command {
public:
    explicit LoginCommand(std::shared_ptr<security::AuthManager> auth_manager);
    void execute(const HttpRequest& request, HttpResponse& response) override;
    
private:
    std::shared_ptr<security::AuthManager> auth_manager_;
};

/**
 * @brief OAuth2 Register command implementation
 */
class RegisterCommand : public Command {
public:
    explicit RegisterCommand(std::shared_ptr<security::AuthManager> auth_manager);
    void execute(const HttpRequest& request, HttpResponse& response) override;
    
private:
    std::shared_ptr<security::AuthManager> auth_manager_;
};

/**
 * @brief OAuth2 Refresh Token command implementation
 */
class RefreshTokenCommand : public Command {
public:
    explicit RefreshTokenCommand(std::shared_ptr<security::AuthManager> auth_manager);
    void execute(const HttpRequest& request, HttpResponse& response) override;
    
private:
    std::shared_ptr<security::AuthManager> auth_manager_;
};

/**
 * @brief OAuth2 Logout command implementation
 */
class LogoutCommand : public Command {
public:
    explicit LogoutCommand(std::shared_ptr<security::AuthManager> auth_manager);
    void execute(const HttpRequest& request, HttpResponse& response) override;
    
private:
    std::shared_ptr<security::AuthManager> auth_manager_;
};

/**
 * @brief Get User Info command implementation
 */
class GetUserInfoCommand : public Command {
public:
    explicit GetUserInfoCommand(std::shared_ptr<security::AuthManager> auth_manager);
    void execute(const HttpRequest& request, HttpResponse& response) override;
    
private:
    std::shared_ptr<security::AuthManager> auth_manager_;
};

/**
 * @brief Request handler using Command pattern and Facade pattern
 * 
 * Acts as a facade for all HTTP request processing
 */
class RequestHandler {
public:
    explicit RequestHandler(std::shared_ptr<FileManager> file_manager, 
                           std::shared_ptr<database::UserRepository> user_repository = nullptr);
    ~RequestHandler() = default;
    
    /**
     * @brief Handle HTTP request
     * @param request HTTP request
     * @param response HTTP response
     */
    void handleRequest(const HttpRequest& request, HttpResponse& response);
    
    /**
     * @brief Register command for specific route
     * @param method HTTP method
     * @param path URL path
     * @param command Command to execute
     */
    void registerCommand(const std::string& method, const std::string& path, 
                        std::unique_ptr<Command> command);

private:
    using RouteKey = std::pair<std::string, std::string>; // method, path
    using RouteMap = std::unordered_map<std::string, std::unique_ptr<Command>>;
    
    std::shared_ptr<FileManager> file_manager_;
    std::shared_ptr<database::UserRepository> user_repository_;
    std::shared_ptr<security::AuthManager> auth_manager_;
    utils::Logger* logger_;
    
    RouteMap routes_;
    
    void setupDefaultRoutes();
    std::string getRouteKey(const std::string& method, const std::string& path) const;
    bool authenticate(const HttpRequest& request) const;
    bool isPublicEndpoint(const std::string& path) const;
    void handleNotFound(HttpResponse& response) const;
    void handleMethodNotAllowed(HttpResponse& response) const;
    void handleInternalError(HttpResponse& response, const std::string& error) const;
};

} // namespace core
} // namespace fileserver
