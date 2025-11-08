#pragma once

#include "connection_pool.h"
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace fileserver {
namespace database {

/**
 * @brief User data model
 */
struct User {
    std::string id;
    std::string username;
    std::string email;
    std::string password_hash;
    std::string salt;
    std::string role;
    bool is_active;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::optional<std::chrono::system_clock::time_point> last_login;
    int login_attempts;
    std::optional<std::chrono::system_clock::time_point> locked_until;
    
    // Convert to/from JSON
    nlohmann::json toJson() const;
    static User fromJson(const nlohmann::json& json);
};

/**
 * @brief User session data model
 */
struct UserSession {
    std::string id;
    std::string user_id;
    std::string token_hash;
    std::chrono::system_clock::time_point expires_at;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_used;
    std::optional<std::string> ip_address;
    std::optional<std::string> user_agent;
    bool is_active;
};

/**
 * @brief Repository for user-related database operations
 * 
 * Implements Repository pattern for user data access with
 * proper error handling, transactions, and security features.
 */
class UserRepository {
public:
    explicit UserRepository(std::shared_ptr<ConnectionPool> pool);
    ~UserRepository() = default;
    
    // User CRUD operations
    
    /**
     * @brief Create a new user
     * @param user User data (id will be generated)
     * @return Created user with generated id, or nullopt on failure
     */
    std::optional<User> createUser(const User& user);
    
    /**
     * @brief Find user by username
     * @param username Username to search for
     * @return User if found, nullopt otherwise
     */
    std::optional<User> findByUsername(const std::string& username);
    
    /**
     * @brief Find user by email
     * @param email Email to search for
     * @return User if found, nullopt otherwise
     */
    std::optional<User> findByEmail(const std::string& email);
    
    /**
     * @brief Find user by ID
     * @param id User ID to search for
     * @return User if found, nullopt otherwise
     */
    std::optional<User> findById(const std::string& id);
    
    /**
     * @brief Find user by token hash (optimized - single JOIN query)
     * @param token_hash Hashed token
     * @return User if found and session is valid, nullopt otherwise
     */
    std::optional<User> findByTokenHash(const std::string& token_hash);
    
    /**
     * @brief Update user data
     * @param user Updated user data
     * @return true if successful, false otherwise
     */
    bool updateUser(const User& user);
    
    /**
     * @brief Delete user by ID
     * @param id User ID to delete
     * @return true if successful, false otherwise
     */
    bool deleteUser(const std::string& id);
    
    /**
     * @brief Get all users with pagination
     * @param offset Number of records to skip
     * @param limit Maximum number of records to return
     * @return Vector of users
     */
    std::vector<User> getAllUsers(int offset = 0, int limit = 100);
    
    /**
     * @brief Get users by role
     * @param role Role to filter by
     * @return Vector of users with specified role
     */
    std::vector<User> getUsersByRole(const std::string& role);
    
    // Authentication operations
    
    /**
     * @brief Authenticate user with username/password
     * @param username Username
     * @param password Plain text password
     * @return User if authentication successful, nullopt otherwise
     */
    std::optional<User> authenticate(const std::string& username, const std::string& password);
    
    /**
     * @brief Update user's last login time
     * @param user_id User ID
     * @param login_time Login timestamp
     * @return true if successful
     */
    bool updateLastLogin(const std::string& user_id, 
                        const std::chrono::system_clock::time_point& login_time);
    
    /**
     * @brief Increment login attempts counter
     * @param user_id User ID
     * @return Current number of login attempts
     */
    int incrementLoginAttempts(const std::string& user_id);
    
    /**
     * @brief Reset login attempts counter
     * @param user_id User ID
     * @return true if successful
     */
    bool resetLoginAttempts(const std::string& user_id);
    
    /**
     * @brief Lock user account until specified time
     * @param user_id User ID
     * @param locked_until Lock expiration time
     * @return true if successful
     */
    bool lockUser(const std::string& user_id, 
                  const std::chrono::system_clock::time_point& locked_until);
    
    /**
     * @brief Unlock user account
     * @param user_id User ID
     * @return true if successful
     */
    bool unlockUser(const std::string& user_id);
    
    // Session management
    
    /**
     * @brief Create a new user session
     * @param session Session data
     * @return Created session with generated id, or nullopt on failure
     */
    std::optional<UserSession> createSession(const UserSession& session);
    
    /**
     * @brief Find session by token hash
     * @param token_hash Hashed token
     * @return Session if found and valid, nullopt otherwise
     */
    std::optional<UserSession> findSessionByToken(const std::string& token_hash);
    
    /**
     * @brief Update session last used time
     * @param session_id Session ID
     * @param last_used Last used timestamp
     * @return true if successful
     */
    bool updateSessionLastUsed(const std::string& session_id,
                              const std::chrono::system_clock::time_point& last_used);
    
    /**
     * @brief Invalidate session
     * @param session_id Session ID
     * @return true if successful
     */
    bool invalidateSession(const std::string& session_id);
    
    /**
     * @brief Invalidate session by token hash (optimized)
     * @param token_hash Hashed token
     * @return true if successful
     */
    bool invalidateSessionByTokenHash(const std::string& token_hash);
    
    /**
     * @brief Invalidate all sessions for user
     * @param user_id User ID
     * @return Number of sessions invalidated
     */
    int invalidateUserSessions(const std::string& user_id);
    
    /**
     * @brief Clean up expired sessions
     * @return Number of sessions cleaned up
     */
    int cleanupExpiredSessions();
    
    // Utility methods
    
    /**
     * @brief Check if username exists
     * @param username Username to check
     * @return true if username exists
     */
    bool usernameExists(const std::string& username);
    
    /**
     * @brief Check if email exists
     * @param email Email to check
     * @return true if email exists
     */
    bool emailExists(const std::string& email);
    
    /**
     * @brief Get user count
     * @return Total number of users
     */
    int getUserCount();
    
    /**
     * @brief Get active session count for user
     * @param user_id User ID
     * @return Number of active sessions
     */
    int getActiveSessionCount(const std::string& user_id);
    
    /**
     * @brief Hash password with salt (public for AuthManager)
     * @param password Plain text password
     * @param salt Salt string
     * @return Hashed password
     */
    std::string hashPassword(const std::string& password, const std::string& salt) const;
    
    /**
     * @brief Generate random salt (public for AuthManager)
     * @return Random salt string
     */
    std::string generateSalt() const;
    
private:
    std::shared_ptr<ConnectionPool> pool_;
    
    /**
     * @brief Verify password against hash
     * @param password Plain text password
     * @param hash Stored password hash
     * @param salt Salt used for hashing
     * @return true if password matches
     */
    bool verifyPassword(const std::string& password, 
                       const std::string& hash, 
                       const std::string& salt) const;
    
    /**
     * @brief Convert PGresult to User object
     * @param result PostgreSQL result
     * @param row Row number
     * @return User object
     */
    User resultToUser(PGresult* result, int row) const;
    
    /**
     * @brief Convert PGresult to UserSession object
     * @param result PostgreSQL result
     * @param row Row number
     * @return UserSession object
     */
    UserSession resultToSession(PGresult* result, int row) const;
    
    /**
     * @brief Parse timestamp from PostgreSQL result
     * @param result PostgreSQL result
     * @param row Row number
     * @param column Column number
     * @return Parsed timestamp
     */
    std::chrono::system_clock::time_point parseTimestamp(PGresult* result, int row, int column) const;
    
    /**
     * @brief Format timestamp for PostgreSQL
     * @param timestamp Timestamp to format
     * @return Formatted timestamp string
     */
    std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) const;
};

} // namespace database
} // namespace fileserver
