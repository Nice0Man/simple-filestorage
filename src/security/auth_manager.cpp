#include "security/auth_manager.h"
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/rand.h>

namespace fileserver {
namespace security {

AuthManager::AuthManager() 
    : use_database_(false), token_cache_(1000, std::chrono::seconds{300}) {
    secret_key_ = generateSecretKey();
}

AuthManager::AuthManager(std::shared_ptr<database::UserRepository> user_repository) 
    : user_repository_(user_repository), use_database_(true), 
      token_cache_(10000, std::chrono::seconds{300}) {
    secret_key_ = generateSecretKey();
}

bool AuthManager::initialize(const std::string& users_file_path) {
    try {
        return loadUsersFromFile(users_file_path);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize AuthManager: " << e.what() << std::endl;
        return false;
    }
}

std::string AuthManager::authenticate(const std::string& username, const std::string& password) {
    try {
        if (use_database_ && user_repository_) {
            // Database-based authentication
            auto user = user_repository_->authenticate(username, password);
            if (user.has_value()) {
                // Generate JWT token
                std::string token = generateToken(username);
                
                // Store token in memory
                auto auth_token = std::make_shared<AuthToken>();
                auth_token->token = token;
                auth_token->username = username;
                auth_token->expires_at = std::chrono::system_clock::now() + token_lifetime_;
                auth_token->is_valid = true;
                
                tokens_[token] = auth_token;
                
                // Also store in database
                database::UserSession session;
                session.user_id = user->id;
                session.token_hash = hashToken(token);
                session.expires_at = auth_token->expires_at;
                session.is_active = true;
                
                auto created_session = user_repository_->createSession(session);
                if (created_session.has_value()) {
                    // Store session ID for later invalidation
                    auth_token->session_id = created_session->id;
                }
                
                return token;
            }
            return "";
        } else {
            // File-based authentication (fallback)
            auto user_it = users_.find(username);
            if (user_it == users_.end()) {
                return ""; // User not found
            }
            
            auto user = user_it->second;
            if (!user->is_active) {
                return ""; // User is inactive
            }
            
            if (!verifyPassword(password, user->password_hash)) {
                return ""; // Invalid password
            }
            
            // Update last login
            user->last_login = std::chrono::system_clock::now();
            
            // Generate JWT token
            std::string token = generateToken(username);
            
            // Store token
            auto auth_token = std::make_shared<AuthToken>();
            auth_token->token = token;
            auth_token->username = username;
            auth_token->expires_at = std::chrono::system_clock::now() + token_lifetime_;
            auth_token->is_valid = true;
            
            tokens_[token] = auth_token;
            
            return token;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Authentication error: " << e.what() << std::endl;
        return "";
    }
}

bool AuthManager::validateToken(const std::string& token) const {
    try {
        auto token_it = tokens_.find(token);
        if (token_it == tokens_.end()) {
            return false;
        }
        
        auto auth_token = token_it->second;
        if (!auth_token->is_valid) {
            return false;
        }
        
        // Check expiration
        if (std::chrono::system_clock::now() > auth_token->expires_at) {
            return false;
        }
        
        // Verify JWT token
        try {
            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{secret_key_})
                .with_issuer("fileserver");
            
            auto decoded = jwt::decode(token);
            verifier.verify(decoded);
            
            return true;
        } catch (const std::exception&) {
            return false;
        }
        
    } catch (const std::exception&) {
        return false;
    }
}

std::shared_ptr<User> AuthManager::getUserByToken(const std::string& token) const {
    try {
        if (!validateToken(token)) {
            return nullptr;
        }
        
        if (use_database_ && user_repository_) {
            // TEMPORARY: Cache disabled for debugging
            std::string token_hash = hashToken(token);
            
            // Optimized single JOIN query
            auto db_user = user_repository_->findByTokenHash(token_hash);
            
            if (!db_user.has_value()) {
                return nullptr;
            }
            
            // Convert database::User to security::User
            auto user = std::make_shared<User>();
            user->username = db_user->username;
            user->password_hash = db_user->password_hash;
            user->role = db_user->role;
            user->is_active = db_user->is_active;
            user->created_at = db_user->created_at;
            user->last_login = db_user->last_login.value_or(std::chrono::system_clock::time_point{});
            
            return user;
        } else {
            // File-based token lookup
            auto token_it = tokens_.find(token);
            if (token_it == tokens_.end()) {
                return nullptr;
            }
            
            std::string username = token_it->second->username;
            auto user_it = users_.find(username);
            if (user_it == users_.end()) {
                return nullptr;
            }
            
            return user_it->second;
        }
        
    } catch (const std::exception&) {
        return nullptr;
    }
}

bool AuthManager::revokeToken(const std::string& token) {
    try {
        // Invalidate in database if using DB (optimized)
        if (use_database_ && user_repository_) {
            std::string token_hash = hashToken(token);
            
            // Use optimized invalidation by token_hash
            user_repository_->invalidateSessionByTokenHash(token_hash);
        }
        
        // Invalidate in memory
        auto token_it = tokens_.find(token);
        if (token_it != tokens_.end()) {
            token_it->second->is_valid = false;
            tokens_.erase(token_it);
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool AuthManager::addUser(const std::string& username, const std::string& password, const std::string& role) {
    try {
        if (use_database_ && user_repository_) {
            // Database-based user creation
            if (user_repository_->usernameExists(username)) {
                return false; // User already exists
            }
            
            // Generate salt and hash password
            std::string salt = user_repository_->generateSalt();
            std::string password_hash = user_repository_->hashPassword(password, salt);
            
            database::User db_user;
            db_user.username = username;
            db_user.password_hash = password_hash;
            db_user.salt = salt;
            db_user.role = role;
            db_user.is_active = true;
            
            auto created = user_repository_->createUser(db_user);
            return created.has_value();
        } else {
            // File-based user creation
            if (users_.find(username) != users_.end()) {
                return false; // User already exists
            }
            
            auto user = std::make_shared<User>();
            user->username = username;
            user->password_hash = hashPassword(password);
            user->role = role;
            user->is_active = true;
            user->created_at = std::chrono::system_clock::now();
            user->last_login = std::chrono::system_clock::time_point{};
            
            users_[username] = user;
            return true;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error adding user: " << e.what() << std::endl;
        return false;
    }
}

bool AuthManager::removeUser(const std::string& username) {
    try {
        auto user_it = users_.find(username);
        if (user_it == users_.end()) {
            return false;
        }
        
        users_.erase(user_it);
        
        // Revoke all tokens for this user
        for (auto it = tokens_.begin(); it != tokens_.end();) {
            if (it->second->username == username) {
                it = tokens_.erase(it);
            } else {
                ++it;
            }
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool AuthManager::changePassword(const std::string& username, const std::string& old_password, 
                                const std::string& new_password) {
    try {
        auto user_it = users_.find(username);
        if (user_it == users_.end()) {
            return false;
        }
        
        auto user = user_it->second;
        if (!verifyPassword(old_password, user->password_hash)) {
            return false;
        }
        
        user->password_hash = hashPassword(new_password);
        
        // Revoke all existing tokens for this user
        for (auto it = tokens_.begin(); it != tokens_.end();) {
            if (it->second->username == username) {
                it = tokens_.erase(it);
            } else {
                ++it;
            }
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

void AuthManager::cleanExpiredTokens() {
    try {
        auto now = std::chrono::system_clock::now();
        
        for (auto it = tokens_.begin(); it != tokens_.end();) {
            if (now > it->second->expires_at || !it->second->is_valid) {
                it = tokens_.erase(it);
            } else {
                ++it;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error cleaning expired tokens: " << e.what() << std::endl;
    }
}

std::string AuthManager::hashPassword(const std::string& password) const {
    // Generate salt
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }
    
    // Create salted password
    std::string salted_password = password + std::string(reinterpret_cast<char*>(salt), sizeof(salt));
    
    // Hash with SHA-256
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted_password.c_str()), 
           salted_password.length(), hash);
    
    // Convert to hex string and prepend salt
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(salt[i]);
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

bool AuthManager::verifyPassword(const std::string& password, const std::string& hash) const {
    try {
        // Check if it's a simple SHA-256 hash (64 hex chars)
        if (hash.length() == 64) {
            // Simple SHA-256 verification for backward compatibility
            unsigned char computed_hash[SHA256_DIGEST_LENGTH];
            SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
                   password.length(), computed_hash);
            
            std::stringstream ss;
            for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(computed_hash[i]);
            }
            
            return ss.str() == hash;
        }
        
        if (hash.length() < 32) {
            return false; // Invalid hash format
        }
        
        // Extract salt (first 32 hex chars = 16 bytes)
        std::string salt_hex = hash.substr(0, 32);
        std::string hash_hex = hash.substr(32);
        
        // Convert salt from hex
        unsigned char salt[16];
        for (int i = 0; i < 16; ++i) {
            std::string byte_str = salt_hex.substr(i * 2, 2);
            salt[i] = static_cast<unsigned char>(std::stoi(byte_str, nullptr, 16));
        }
        
        // Create salted password
        std::string salted_password = password + std::string(reinterpret_cast<char*>(salt), sizeof(salt));
        
        // Hash with SHA-256
        unsigned char computed_hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(salted_password.c_str()), 
               salted_password.length(), computed_hash);
        
        // Convert to hex string
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(computed_hash[i]);
        }
        
        return ss.str() == hash_hex;
        
    } catch (const std::exception&) {
        return false;
    }
}

std::string AuthManager::generateToken(const std::string& username) const {
    try {
        auto now = std::chrono::system_clock::now();
        auto expires = now + token_lifetime_;
        
        // Generate unique token ID to ensure tokens are always different
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;
        std::string jti = std::to_string(dis(gen)) + std::to_string(
            std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count()
        );
        
        auto token = jwt::create()
            .set_issuer("fileserver")
            .set_type("JWT")
            .set_id(jti)  // Add unique JWT ID
            .set_issued_at(now)
            .set_expires_at(expires)
            .set_payload_claim("username", jwt::claim(username))
            .sign(jwt::algorithm::hs256{secret_key_});
        
        return token;
        
    } catch (const std::exception& e) {
        std::cerr << "Error generating token: " << e.what() << std::endl;
        return "";
    }
}

std::string AuthManager::generateSecretKey() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    
    return ss.str();
}

std::string AuthManager::hashToken(const std::string& token) const {
    // Hash token with SHA-256 for database storage
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(token.c_str()), 
           token.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

bool AuthManager::loadUsersFromFile(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            // Create default admin user if file doesn't exist
            addUser("admin", "admin123", "admin");
            return saveUsersToFile(file_path);
        }
        
        nlohmann::json users_json;
        file >> users_json;
        
        for (const auto& user_data : users_json["users"]) {
            auto user = std::make_shared<User>();
            user->username = user_data["username"];
            user->password_hash = user_data["password_hash"];
            user->role = user_data["role"];
            user->is_active = user_data.value("is_active", true);
            
            // Parse timestamps
            if (user_data.contains("created_at")) {
                auto created_time = std::chrono::system_clock::from_time_t(user_data["created_at"]);
                user->created_at = created_time;
            }
            
            users_[user->username] = user;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading users: " << e.what() << std::endl;
        return false;
    }
}

bool AuthManager::saveUsersToFile(const std::string& file_path) const {
    try {
        nlohmann::json users_json;
        users_json["users"] = nlohmann::json::array();
        
        for (const auto& [username, user] : users_) {
            nlohmann::json user_data;
            user_data["username"] = user->username;
            user_data["password_hash"] = user->password_hash;
            user_data["role"] = user->role;
            user_data["is_active"] = user->is_active;
            user_data["created_at"] = std::chrono::system_clock::to_time_t(user->created_at);
            
            users_json["users"].push_back(user_data);
        }
        
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        file << users_json.dump(4);
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving users: " << e.what() << std::endl;
        return false;
    }
}

utils::LRUCache<utils::CachedUserData>::Stats AuthManager::getCacheStats() const {
    return token_cache_.getStats();
}

} // namespace security
} // namespace fileserver
