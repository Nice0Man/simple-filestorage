#include "database/user_repository.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <openssl/sha.h>
#include <cstring>

namespace fileserver {
namespace database {

// User JSON serialization
nlohmann::json User::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["username"] = username;
    json["email"] = email;
    json["role"] = role;
    json["is_active"] = is_active;
    json["login_attempts"] = login_attempts;
    
    // Convert timestamps to ISO 8601 strings
    auto time_to_string = [](const std::chrono::system_clock::time_point& tp) {
        auto time_t = std::chrono::system_clock::to_time_t(tp);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    };
    
    json["created_at"] = time_to_string(created_at);
    json["updated_at"] = time_to_string(updated_at);
    
    if (last_login.has_value()) {
        json["last_login"] = time_to_string(last_login.value());
    }
    
    if (locked_until.has_value()) {
        json["locked_until"] = time_to_string(locked_until.value());
    }
    
    return json;
}

User User::fromJson(const nlohmann::json& json) {
    User user;
    user.id = json.value("id", "");
    user.username = json.value("username", "");
    user.email = json.value("email", "");
    user.password_hash = json.value("password_hash", "");
    user.salt = json.value("salt", "");
    user.role = json.value("role", "user");
    user.is_active = json.value("is_active", true);
    user.login_attempts = json.value("login_attempts", 0);
    
    // Parse timestamps from ISO 8601 format
    auto parse_iso8601 = [](const std::string& iso_str) -> std::chrono::system_clock::time_point {
        if (iso_str.empty()) {
            return std::chrono::system_clock::now();
        }
        
        std::tm tm = {};
        std::istringstream ss(iso_str);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        
        if (ss.fail()) {
            return std::chrono::system_clock::now();
        }
        
        std::time_t time_c = std::mktime(&tm);
        return std::chrono::system_clock::from_time_t(time_c);
    };
    
    if (json.contains("created_at")) {
        user.created_at = parse_iso8601(json["created_at"].get<std::string>());
    } else {
        user.created_at = std::chrono::system_clock::now();
    }
    
    if (json.contains("updated_at")) {
        user.updated_at = parse_iso8601(json["updated_at"].get<std::string>());
    } else {
        user.updated_at = std::chrono::system_clock::now();
    }
    
    if (json.contains("last_login") && !json["last_login"].is_null()) {
        user.last_login = parse_iso8601(json["last_login"].get<std::string>());
    }
    
    if (json.contains("locked_until") && !json["locked_until"].is_null()) {
        user.locked_until = parse_iso8601(json["locked_until"].get<std::string>());
    }
    
    return user;
}

// UserRepository implementation
UserRepository::UserRepository(std::shared_ptr<ConnectionPool> pool) : pool_(pool) {}

std::optional<User> UserRepository::createUser(const User& user) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return std::nullopt;
    }
    
    // Generate salt and hash password if not provided
    std::string salt = user.salt.empty() ? generateSalt() : user.salt;
    std::string password_hash = user.password_hash;
    
    const char* query = R"(
        INSERT INTO users (username, email, password_hash, salt, role, is_active)
        VALUES ($1, $2, $3, $4, $5, $6)
        RETURNING id, username, email, password_hash, salt, role, is_active,
                  created_at, updated_at, last_login, login_attempts, locked_until
    )";
    
    const char* params[6] = {
        user.username.c_str(),
        user.email.c_str(),
        password_hash.c_str(),
        salt.c_str(),
        user.role.c_str(),
        user.is_active ? "true" : "false"
    };
    
    PGresult* result = PQexecParams(conn->get(), query, 6, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    User created_user = resultToUser(result, 0);
    PQclear(result);
    
    return created_user;
}

std::optional<User> UserRepository::findByUsername(const std::string& username) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return std::nullopt;
    }
    
    const char* query = R"(
        SELECT id, username, email, password_hash, salt, role, is_active,
               created_at, updated_at, last_login, login_attempts, locked_until
        FROM users
        WHERE username = $1 AND is_active = true
    )";
    
    const char* params[1] = { username.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    User user = resultToUser(result, 0);
    PQclear(result);
    
    return user;
}

std::optional<User> UserRepository::findByEmail(const std::string& email) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return std::nullopt;
    }
    
    const char* query = R"(
        SELECT id, username, email, password_hash, salt, role, is_active,
               created_at, updated_at, last_login, login_attempts, locked_until
        FROM users
        WHERE email = $1 AND is_active = true
    )";
    
    const char* params[1] = { email.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    User user = resultToUser(result, 0);
    PQclear(result);
    
    return user;
}

std::optional<User> UserRepository::findById(const std::string& id) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return std::nullopt;
    }
    
    const char* query = R"(
        SELECT id, username, email, password_hash, salt, role, is_active,
               created_at, updated_at, last_login, login_attempts, locked_until
        FROM users
        WHERE id = $1
    )";
    
    const char* params[1] = { id.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    User user = resultToUser(result, 0);
    PQclear(result);
    
    return user;
}

std::optional<User> UserRepository::findByTokenHash(const std::string& token_hash) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return std::nullopt;
    }
    
    // Оптимизированный запрос - один JOIN вместо двух отдельных запросов
    const char* query = R"(
        SELECT u.id, u.username, u.email, u.password_hash, u.salt, u.role, u.is_active,
               u.created_at, u.updated_at, u.last_login, u.login_attempts, u.locked_until
        FROM users u
        INNER JOIN user_sessions s ON s.user_id = u.id
        WHERE s.token_hash = $1 
          AND s.is_active = true 
          AND s.expires_at > CURRENT_TIMESTAMP
        LIMIT 1
    )";
    
    const char* params[1] = { token_hash.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    User user = resultToUser(result, 0);
    PQclear(result);
    
    return user;
}

std::optional<User> UserRepository::authenticate(const std::string& username, const std::string& password) {
    auto user = findByUsername(username);
    if (!user.has_value()) {
        return std::nullopt;
    }
    
    // Check if account is locked
    if (user->locked_until.has_value() && 
        user->locked_until.value() > std::chrono::system_clock::now()) {
        return std::nullopt;
    }
    
    // Verify password
    if (!verifyPassword(password, user->password_hash, user->salt)) {
        // Increment login attempts
        incrementLoginAttempts(user->id);
        return std::nullopt;
    }
    
    // Reset login attempts on successful authentication
    resetLoginAttempts(user->id);
    updateLastLogin(user->id, std::chrono::system_clock::now());
    
    return user;
}

bool UserRepository::updateLastLogin(const std::string& user_id, 
                                    const std::chrono::system_clock::time_point& login_time) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return false;
    }
    
    std::string timestamp = formatTimestamp(login_time);
    
    const char* query = "UPDATE users SET last_login = $1 WHERE id = $2";
    const char* params[2] = { timestamp.c_str(), user_id.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 2, nullptr, params, nullptr, nullptr, 0);
    bool success = PQresultStatus(result) == PGRES_COMMAND_OK;
    PQclear(result);
    
    return success;
}

int UserRepository::incrementLoginAttempts(const std::string& user_id) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return -1;
    }
    
    const char* query = R"(
        UPDATE users 
        SET login_attempts = login_attempts + 1 
        WHERE id = $1 
        RETURNING login_attempts
    )";
    
    const char* params[1] = { user_id.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return -1;
    }
    
    int attempts = std::atoi(PQgetvalue(result, 0, 0));
    PQclear(result);
    
    return attempts;
}

bool UserRepository::resetLoginAttempts(const std::string& user_id) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return false;
    }
    
    const char* query = "UPDATE users SET login_attempts = 0 WHERE id = $1";
    const char* params[1] = { user_id.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    bool success = PQresultStatus(result) == PGRES_COMMAND_OK;
    PQclear(result);
    
    return success;
}

std::string UserRepository::hashPassword(const std::string& password, const std::string& salt) const {
    std::string salted_password = password + salt;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted_password.c_str()), 
           salted_password.length(), hash);
    
    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return oss.str();
}

std::string UserRepository::generateSalt() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    
    return oss.str();
}

bool UserRepository::verifyPassword(const std::string& password, 
                                   const std::string& hash, 
                                   const std::string& salt) const {
    std::string computed_hash = hashPassword(password, salt);
    return computed_hash == hash;
}

User UserRepository::resultToUser(PGresult* result, int row) const {
    User user;
    user.id = PQgetvalue(result, row, 0);
    user.username = PQgetvalue(result, row, 1);
    user.email = PQgetvalue(result, row, 2);
    user.password_hash = PQgetvalue(result, row, 3);
    user.salt = PQgetvalue(result, row, 4);
    user.role = PQgetvalue(result, row, 5);
    user.is_active = std::string(PQgetvalue(result, row, 6)) == "t";
    user.created_at = parseTimestamp(result, row, 7);
    user.updated_at = parseTimestamp(result, row, 8);
    
    if (!PQgetisnull(result, row, 9)) {
        user.last_login = parseTimestamp(result, row, 9);
    }
    
    user.login_attempts = std::atoi(PQgetvalue(result, row, 10));
    
    if (!PQgetisnull(result, row, 11)) {
        user.locked_until = parseTimestamp(result, row, 11);
    }
    
    return user;
}

UserSession UserRepository::resultToSession(PGresult* result, int row) const {
    UserSession session;
    session.id = PQgetvalue(result, row, 0);
    session.user_id = PQgetvalue(result, row, 1);
    session.token_hash = PQgetvalue(result, row, 2);
    session.expires_at = parseTimestamp(result, row, 3);
    session.created_at = parseTimestamp(result, row, 4);
    session.last_used = parseTimestamp(result, row, 5);
    
    if (!PQgetisnull(result, row, 6)) {
        session.ip_address = PQgetvalue(result, row, 6);
    }
    
    if (!PQgetisnull(result, row, 7)) {
        session.user_agent = PQgetvalue(result, row, 7);
    }
    
    session.is_active = std::string(PQgetvalue(result, row, 8)) == "t";
    
    return session;
}

std::chrono::system_clock::time_point UserRepository::parseTimestamp(PGresult* result, int row, int column) const {
    if (PQgetisnull(result, row, column)) {
        return std::chrono::system_clock::now();
    }
    
    const char* timestamp_str = PQgetvalue(result, row, column);
    
    // Parse PostgreSQL timestamp format: "YYYY-MM-DD HH:MM:SS.microseconds+TZ"
    // Example: "2024-11-09 10:30:15.123456+00"
    std::tm tm = {};
    std::istringstream ss(timestamp_str);
    
    // Parse date and time
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    if (ss.fail()) {
        // Fallback to current time if parsing fails
        return std::chrono::system_clock::now();
    }
    
    // Convert to time_point
    std::time_t time_c = std::mktime(&tm);
    auto time_point = std::chrono::system_clock::from_time_t(time_c);
    
    // Parse microseconds if present
    std::string remaining = timestamp_str;
    size_t dot_pos = remaining.find('.');
    if (dot_pos != std::string::npos) {
        size_t plus_pos = remaining.find('+', dot_pos);
        size_t minus_pos = remaining.find('-', dot_pos);
        size_t tz_pos = std::min(plus_pos, minus_pos);
        
        if (tz_pos != std::string::npos) {
            std::string microseconds_str = remaining.substr(dot_pos + 1, tz_pos - dot_pos - 1);
            try {
                long microseconds = std::stol(microseconds_str);
                // Pad or truncate to 6 digits
                while (microseconds_str.length() < 6) {
                    microseconds *= 10;
                }
                time_point += std::chrono::microseconds(microseconds);
            } catch (...) {
                // Ignore microseconds if parsing fails
            }
        }
    }
    
    return time_point;
}

std::string UserRepository::formatTimestamp(const std::chrono::system_clock::time_point& timestamp) const {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool UserRepository::usernameExists(const std::string& username) {
    return findByUsername(username).has_value();
}

bool UserRepository::emailExists(const std::string& email) {
    return findByEmail(email).has_value();
}

int UserRepository::getUserCount() {
    auto conn = pool_->getConnection();
    if (!conn) {
        return -1;
    }
    
    const char* query = "SELECT COUNT(*) FROM users WHERE is_active = true";
    
    PGresult* result = PQexec(conn->get(), query);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return -1;
    }
    
    int count = std::atoi(PQgetvalue(result, 0, 0));
    PQclear(result);
    
    return count;
}

// Session management methods
std::optional<UserSession> UserRepository::createSession(const UserSession& session) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return std::nullopt;
    }
    
    std::string expires_at = formatTimestamp(session.expires_at);
    std::string ip = session.ip_address.value_or("");
    std::string ua = session.user_agent.value_or("");
    
    const char* query = R"(
        INSERT INTO user_sessions (user_id, token_hash, expires_at, ip_address, user_agent, is_active)
        VALUES ($1, $2, $3, $4, $5, $6)
        RETURNING id, user_id, token_hash, expires_at, created_at, last_used, 
                  ip_address, user_agent, is_active
    )";
    
    const char* params[6] = {
        session.user_id.c_str(),
        session.token_hash.c_str(),
        expires_at.c_str(),
        ip.empty() ? nullptr : ip.c_str(),
        ua.empty() ? nullptr : ua.c_str(),
        session.is_active ? "true" : "false"
    };
    
    PGresult* result = PQexecParams(conn->get(), query, 6, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    UserSession created_session = resultToSession(result, 0);
    PQclear(result);
    
    return created_session;
}

std::optional<UserSession> UserRepository::findSessionByToken(const std::string& token_hash) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return std::nullopt;
    }
    
    const char* query = R"(
        SELECT id, user_id, token_hash, expires_at, created_at, last_used, 
               ip_address, user_agent, is_active
        FROM user_sessions
        WHERE token_hash = $1 AND is_active = true AND expires_at > CURRENT_TIMESTAMP
    )";
    
    const char* params[1] = { token_hash.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    UserSession session = resultToSession(result, 0);
    PQclear(result);
    
    return session;
}

bool UserRepository::updateSessionLastUsed(const std::string& session_id,
                                          const std::chrono::system_clock::time_point& last_used) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return false;
    }
    
    std::string timestamp = formatTimestamp(last_used);
    
    const char* query = "UPDATE user_sessions SET last_used = $1 WHERE id = $2";
    const char* params[2] = { timestamp.c_str(), session_id.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 2, nullptr, params, nullptr, nullptr, 0);
    bool success = PQresultStatus(result) == PGRES_COMMAND_OK;
    PQclear(result);
    
    return success;
}

bool UserRepository::invalidateSession(const std::string& session_id) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return false;
    }
    
    const char* query = "UPDATE user_sessions SET is_active = false WHERE id = $1";
    const char* params[1] = { session_id.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    bool success = PQresultStatus(result) == PGRES_COMMAND_OK;
    PQclear(result);
    
    return success;
}

bool UserRepository::invalidateSessionByTokenHash(const std::string& token_hash) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return false;
    }
    
    // Оптимизированный метод - деактивация по token_hash без поиска session_id
    const char* query = "UPDATE user_sessions SET is_active = false WHERE token_hash = $1";
    const char* params[1] = { token_hash.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    bool success = PQresultStatus(result) == PGRES_COMMAND_OK;
    PQclear(result);
    
    return success;
}

int UserRepository::invalidateUserSessions(const std::string& user_id) {
    auto conn = pool_->getConnection();
    if (!conn) {
        return 0;
    }
    
    const char* query = "UPDATE user_sessions SET is_active = false WHERE user_id = $1 AND is_active = true";
    const char* params[1] = { user_id.c_str() };
    
    PGresult* result = PQexecParams(conn->get(), query, 1, nullptr, params, nullptr, nullptr, 0);
    
    int affected = 0;
    if (PQresultStatus(result) == PGRES_COMMAND_OK) {
        const char* affected_str = PQcmdTuples(result);
        if (affected_str && strlen(affected_str) > 0) {
            affected = std::atoi(affected_str);
        }
    }
    
    PQclear(result);
    return affected;
}

int UserRepository::cleanupExpiredSessions() {
    auto conn = pool_->getConnection();
    if (!conn) {
        return -1;
    }
    
    PGresult* result = PQexec(conn->get(), "SELECT clean_expired_sessions()");
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        PQclear(result);
        return -1;
    }
    
    int count = std::atoi(PQgetvalue(result, 0, 0));
    PQclear(result);
    
    return count;
}

} // namespace database
} // namespace fileserver
