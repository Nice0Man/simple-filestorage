#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>
#include "database/user_repository.h"
#include "utils/token_cache.h"

namespace fileserver {
namespace security {

/**
 * @brief User information structure
 */
struct User {
    std::string username;
    std::string password_hash;
    std::string role;
    bool is_active;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_login;
};

/**
 * @brief Authentication token structure
 */
struct AuthToken {
    std::string token;
    std::string username;
    std::chrono::system_clock::time_point expires_at;
    bool is_valid;
    std::string session_id;  // Database session ID for invalidation
};

/**
 * @brief Authentication manager class
 */
class AuthManager {
public:
    // Constructor for file-based authentication
    AuthManager();
    
    // Constructor for database-based authentication
    explicit AuthManager(std::shared_ptr<database::UserRepository> user_repository);
    
    ~AuthManager() = default;
    
    /**
     * @brief Initialize authentication manager with file
     * @param users_file_path Path to users configuration file
     * @return true if initialization successful
     */
    bool initialize(const std::string& users_file_path);
    
    /**
     * @brief Authenticate user with username and password
     * @param username Username
     * @param password Password
     * @return Authentication token if successful, empty string otherwise
     */
    std::string authenticate(const std::string& username, const std::string& password);
    
    /**
     * @brief Validate authentication token
     * @param token Authentication token
     * @return true if token is valid
     */
    bool validateToken(const std::string& token) const;
    
    /**
     * @brief Get user information by token
     * @param token Authentication token
     * @return User information if token is valid
     */
    std::shared_ptr<User> getUserByToken(const std::string& token) const;
    
    /**
     * @brief Revoke authentication token
     * @param token Authentication token
     * @return true if token was revoked
     */
    bool revokeToken(const std::string& token);
    
    /**
     * @brief Add new user
     * @param username Username
     * @param password Password
     * @param role User role
     * @return true if user was added successfully
     */
    bool addUser(const std::string& username, const std::string& password, const std::string& role);
    
    /**
     * @brief Remove user
     * @param username Username
     * @return true if user was removed successfully
     */
    bool removeUser(const std::string& username);
    
    /**
     * @brief Change user password
     * @param username Username
     * @param old_password Old password
     * @param new_password New password
     * @return true if password was changed successfully
     */
    bool changePassword(const std::string& username, const std::string& old_password, 
                       const std::string& new_password);
    
    /**
     * @brief Clean expired tokens
     */
    void cleanExpiredTokens();

    /**
     * @brief Get cache statistics
     */
    utils::LRUCache<utils::CachedUserData>::Stats getCacheStats() const;
    
private:
    // File-based authentication data
    std::unordered_map<std::string, std::shared_ptr<User>> users_;
    std::unordered_map<std::string, std::shared_ptr<AuthToken>> tokens_;
    
    // Database-based authentication
    std::shared_ptr<database::UserRepository> user_repository_;
    bool use_database_;
    
    // Performance optimization - token cache
    mutable utils::LRUCache<utils::CachedUserData> token_cache_;
    
    std::string secret_key_;
    std::chrono::minutes token_lifetime_{60}; // 1 hour default
    
    std::string hashPassword(const std::string& password) const;
    bool verifyPassword(const std::string& password, const std::string& hash) const;
    std::string generateToken(const std::string& username) const;
    std::string generateSecretKey() const;
    std::string hashToken(const std::string& token) const;
    bool loadUsersFromFile(const std::string& file_path);
    bool saveUsersToFile(const std::string& file_path) const;
};

} // namespace security
} // namespace fileserver
