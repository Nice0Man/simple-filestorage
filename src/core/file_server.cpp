#include "core/file_server.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <httplib.h>
#include "database/connection_pool.h"
#include "database/user_repository.h"

namespace fileserver {
namespace core {

// Static member definitions
std::unique_ptr<FileServer> FileServer::instance_ = nullptr;
std::mutex FileServer::instance_mutex_;

FileServer& FileServer::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (instance_ == nullptr) {
        instance_ = std::unique_ptr<FileServer>(new FileServer());
    }
    return *instance_;
}

bool FileServer::initialize(const std::string& config_path) {
    try {
        // Initialize configuration manager
        config_manager_ = std::make_unique<utils::ConfigManager>();
        if (!config_manager_->loadFromFile(config_path)) {
            std::cerr << "Failed to load configuration from: " << config_path << std::endl;
            return false;
        }
        
        // Get logger instance (already initialized in main)
        logger_ = &utils::Logger::getInstance();
        
        // Get server configuration
        host_ = config_manager_->getString("host", "0.0.0.0");
        port_ = config_manager_->getInt("port", 8080);
        
        // Initialize database connection pool
        database::ConnectionPool::Config db_config;
        db_config.host = config_manager_->getString("database.host", "localhost");
        db_config.port = config_manager_->getInt("database.port", 5432);
        db_config.database = config_manager_->getString("database.name", "fileserver");
        db_config.username = config_manager_->getString("database.username", "fileserver");
        db_config.password = config_manager_->getString("database.password", "fileserver123");
        db_config.min_connections = config_manager_->getInt("database.min_connections", 5);
        db_config.max_connections = config_manager_->getInt("database.max_connections", 20);
        db_config.enable_ssl = config_manager_->getBool("database.enable_ssl", false);
        
        connection_pool_ = std::make_shared<database::ConnectionPool>(db_config);
        
        // Test database connection
        if (!connection_pool_->healthCheck()) {
            logger_->error("Failed to connect to PostgreSQL database");
            return false;
        }
        
        // Initialize user repository
        user_repository_ = std::make_shared<database::UserRepository>(connection_pool_);
        
        // Initialize file manager
        std::string root_directory = config_manager_->getString("root_directory", "./files");
        auto file_manager = std::make_shared<FileManager>(root_directory);
        
        // Initialize request handler with database support
        request_handler_ = std::make_unique<RequestHandler>(file_manager, user_repository_);
        
        logger_->info("FileServer initialized successfully");
        logger_->info("Host: " + host_ + ", Port: " + std::to_string(port_));
        logger_->info("Root directory: " + root_directory);
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception during initialization: " << e.what() << std::endl;
        return false;
    }
}

bool FileServer::start() {
    if (running_) {
        logger_->warning("Server is already running");
        return false;
    }
    
    try {
        running_ = true;
        
        // Start server thread
        server_thread_ = std::thread(&FileServer::serverLoop, this);
        
        logger_->info("Server started on " + host_ + ":" + std::to_string(port_));
        return true;
        
    } catch (const std::exception& e) {
        logger_->error("Failed to start server: " + std::string(e.what()));
        running_ = false;
        return false;
    }
}

void FileServer::stop() {
    if (!running_) {
        return;
    }
    
    logger_->info("Stopping server...");
    running_ = false;
    
    // Wait for server thread to finish
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    logger_->info("Server stopped");
}

bool FileServer::isRunning() const {
    return running_;
}

const utils::ConfigManager& FileServer::getConfig() const {
    return *config_manager_;
}

void FileServer::serverLoop() {
    logger_->info("Server loop started");
    
    try {
        httplib::Server server;
        
        // Enable CORS
        server.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            return httplib::Server::HandlerResponse::Unhandled;
        });
        
        // Handle OPTIONS requests
        server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            return;
        });
        
        // Health check endpoint
        server.Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "GET";
            custom_req.path = "/health";
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // Swagger UI endpoint
        server.Get("/docs", [this](const httplib::Request& req, httplib::Response& res) {
            std::ifstream file("docs/swagger_ui.html");
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
                res.set_content(content, "text/html");
            } else {
                res.status = 404;
                res.set_content("Swagger UI not found", "text/plain");
            }
        });
        
        // Swagger YAML endpoint
        server.Get("/swagger.yaml", [this](const httplib::Request& req, httplib::Response& res) {
            std::ifstream file("docs/swagger.yaml");
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
                res.set_content(content, "application/x-yaml");
            } else {
                res.status = 404;
                res.set_content("Swagger YAML not found", "text/plain");
            }
        });
        
        // OAuth2 / Authentication endpoints
        
        // Login endpoint
        server.Post("/api/v1/auth/login", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "POST";
            custom_req.path = "/api/v1/auth/login";
            custom_req.body = req.body;
            
            // Copy headers
            for (const auto& [key, value] : req.headers) {
                custom_req.headers[key] = value;
            }
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // Register endpoint
        server.Post("/api/v1/auth/register", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "POST";
            custom_req.path = "/api/v1/auth/register";
            custom_req.body = req.body;
            
            // Copy headers
            for (const auto& [key, value] : req.headers) {
                custom_req.headers[key] = value;
            }
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // Refresh token endpoint
        server.Post("/api/v1/auth/refresh", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "POST";
            custom_req.path = "/api/v1/auth/refresh";
            custom_req.body = req.body;
            
            // Copy headers
            for (const auto& [key, value] : req.headers) {
                custom_req.headers[key] = value;
            }
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // Logout endpoint
        server.Post("/api/v1/auth/logout", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "POST";
            custom_req.path = "/api/v1/auth/logout";
            custom_req.body = req.body;
            
            // Copy headers
            for (const auto& [key, value] : req.headers) {
                custom_req.headers[key] = value;
            }
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // Get user info endpoint
        server.Get("/api/v1/auth/me", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "GET";
            custom_req.path = "/api/v1/auth/me";
            
            // Copy headers
            for (const auto& [key, value] : req.headers) {
                custom_req.headers[key] = value;
            }
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // File listing
        server.Get("/api/v1/files", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "GET";
            custom_req.path = "/api/v1/files";
            
            // Copy headers
            for (const auto& [key, value] : req.headers) {
                custom_req.headers[key] = value;
            }
            
            // Add query parameters to headers for simplicity
            for (const auto& [key, value] : req.params) {
                custom_req.headers[key] = value;
            }
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // File upload
        server.Post("/api/v1/files/upload", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "POST";
            custom_req.path = "/api/v1/files/upload";
            custom_req.body = req.body;
            
            // Copy headers
            for (const auto& [key, value] : req.headers) {
                custom_req.headers[key] = value;
            }
            
            // Handle multipart form data
            if (req.is_multipart_form_data()) {
                for (const auto& file : req.files) {
                    custom_req.form_data["file"] = file.second.content;
                    custom_req.form_data["filename"] = file.second.filename;
                }
                for (const auto& param : req.params) {
                    custom_req.form_data[param.first] = param.second;
                }
            }
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // File download
        server.Get(R"(/api/v1/files/download/(.*))", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "GET";
            custom_req.path = "/api/v1/files/download/" + req.matches[1].str();
            
            // Copy headers
            for (const auto& [key, value] : req.headers) {
                custom_req.headers[key] = value;
            }
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // File deletion
        server.Delete(R"(/api/v1/files/(.*))", [this](const httplib::Request& req, httplib::Response& res) {
            HttpRequest custom_req;
            custom_req.method = "DELETE";
            custom_req.path = "/api/v1/files/" + req.matches[1].str();
            
            // Copy headers
            for (const auto& [key, value] : req.headers) {
                custom_req.headers[key] = value;
            }
            
            HttpResponse custom_res;
            request_handler_->handleRequest(custom_req, custom_res);
            
            res.status = custom_res.status_code;
            res.body = custom_res.body;
            // Copy headers, skip Content-Length (httplib sets it automatically)
            for (const auto& [key, value] : custom_res.headers) {
                if (key != "Content-Length") {
                    res.set_header(key, value);
                }
            }
        });
        
        // Authentication endpoint
        server.Post("/api/v1/auth/login", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                nlohmann::json request_json = nlohmann::json::parse(req.body);
                std::string username = request_json["username"];
                std::string password = request_json["password"];
                
                // Get auth manager from request handler (simplified)
                // In production, this should be properly structured
                std::string token = ""; // auth_manager_->authenticate(username, password);
                
                nlohmann::json response_json;
                if (!token.empty()) {
                    response_json["success"] = true;
                    response_json["token"] = token;
                    res.status = 200;
                } else {
                    response_json["success"] = false;
                    response_json["error"] = "Invalid credentials";
                    res.status = 401;
                }
                
                res.set_header("Content-Type", "application/json");
                res.body = response_json.dump();
                
            } catch (const std::exception& e) {
                nlohmann::json error_response;
                error_response["success"] = false;
                error_response["error"] = "Invalid request format";
                
                res.status = 400;
                res.set_header("Content-Type", "application/json");
                res.body = error_response.dump();
            }
        });
        
        // Set server configuration
        server.set_keep_alive_max_count(config_manager_->getInt("server.max_connections", 1000));
        server.set_read_timeout(config_manager_->getInt("server.timeout_seconds", 30));
        server.set_write_timeout(config_manager_->getInt("server.timeout_seconds", 30));
        
        logger_->info("Starting HTTP server on " + host_ + ":" + std::to_string(port_));
        
        // Start server (this blocks until server stops)
        if (!server.listen(host_, port_)) {
            logger_->error("Failed to start HTTP server");
            running_ = false;
        }
        
    } catch (const std::exception& e) {
        logger_->error("Error in server loop: " + std::string(e.what()));
        running_ = false;
    }
    
    logger_->info("Server loop ended");
}

void FileServer::setupRoutes() {
    // Routes are set up in RequestHandler constructor
    // This method could be used for additional route configuration
}

} // namespace core
} // namespace fileserver
