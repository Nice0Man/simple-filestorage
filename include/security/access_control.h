#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace fileserver {
namespace security {

/**
 * @brief Permission enumeration
 */
enum class Permission {
    READ = 1,
    WRITE = 2,
    DELETE = 4,
    ADMIN = 8
};

/**
 * @brief Role-based access control class
 */
class AccessControl {
public:
    AccessControl() = default;
    ~AccessControl() = default;
    
    /**
     * @brief Initialize access control with configuration
     * @param config_path Path to access control configuration
     * @return true if initialization successful
     */
    bool initialize(const std::string& config_path);
    
    /**
     * @brief Check if user has permission for resource
     * @param username Username
     * @param resource Resource path
     * @param permission Required permission
     * @return true if user has permission
     */
    bool hasPermission(const std::string& username, const std::string& resource, 
                      Permission permission) const;
    
    /**
     * @brief Check if role has permission
     * @param role User role
     * @param permission Required permission
     * @return true if role has permission
     */
    bool roleHasPermission(const std::string& role, Permission permission) const;
    
    /**
     * @brief Add role permission
     * @param role User role
     * @param permission Permission to add
     */
    void addRolePermission(const std::string& role, Permission permission);
    
    /**
     * @brief Remove role permission
     * @param role User role
     * @param permission Permission to remove
     */
    void removeRolePermission(const std::string& role, Permission permission);
    
    /**
     * @brief Set user role
     * @param username Username
     * @param role User role
     */
    void setUserRole(const std::string& username, const std::string& role);
    
    /**
     * @brief Get user role
     * @param username Username
     * @return User role
     */
    std::string getUserRole(const std::string& username) const;
    
    /**
     * @brief Check if path is accessible
     * @param path File path
     * @param username Username
     * @return true if path is accessible
     */
    bool isPathAccessible(const std::string& path, const std::string& username) const;
    
    /**
     * @brief Add path restriction
     * @param path File path pattern
     * @param allowed_roles Roles allowed to access this path
     */
    void addPathRestriction(const std::string& path, const std::vector<std::string>& allowed_roles);

private:
    std::unordered_map<std::string, int> role_permissions_; // role -> permission bitmask
    std::unordered_map<std::string, std::string> user_roles_; // username -> role
    std::unordered_map<std::string, std::vector<std::string>> path_restrictions_; // path -> allowed roles
    
    bool loadConfigFromFile(const std::string& config_path);
    void setupDefaultConfiguration();
    bool matchesPattern(const std::string& path, const std::string& pattern) const;
    int permissionToInt(Permission permission) const;
};

} // namespace security
} // namespace fileserver
