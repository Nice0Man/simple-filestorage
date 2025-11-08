#include "security/access_control.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <nlohmann/json.hpp>

namespace fileserver {
namespace security {

bool AccessControl::initialize(const std::string &config_path) {
  try {
    return loadConfigFromFile(config_path);
  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize AccessControl: " << e.what()
              << std::endl;
    return false;
  }
}

bool AccessControl::hasPermission(const std::string &username,
                                  const std::string &resource,
                                  Permission permission) const {
  try {
    // Get user role
    auto user_it = user_roles_.find(username);
    if (user_it == user_roles_.end()) {
      return false; // User not found
    }

    std::string role = user_it->second;

    // Check role permissions
    if (!roleHasPermission(role, permission)) {
      return false;
    }

    // Check path-specific restrictions
    return isPathAccessible(resource, username);

  } catch (const std::exception &) {
    return false;
  }
}

bool AccessControl::roleHasPermission(const std::string &role,
                                      Permission permission) const {
  try {
    auto role_it = role_permissions_.find(role);
    if (role_it == role_permissions_.end()) {
      return false;
    }

    int role_perms = role_it->second;
    int required_perm = permissionToInt(permission);

    return (role_perms & required_perm) != 0;

  } catch (const std::exception &) {
    return false;
  }
}

void AccessControl::addRolePermission(const std::string &role,
                                      Permission permission) {
  try {
    int perm_int = permissionToInt(permission);
    role_permissions_[role] |= perm_int;
  } catch (const std::exception &e) {
    std::cerr << "Error adding role permission: " << e.what() << std::endl;
  }
}

void AccessControl::removeRolePermission(const std::string &role,
                                         Permission permission) {
  try {
    int perm_int = permissionToInt(permission);
    auto role_it = role_permissions_.find(role);
    if (role_it != role_permissions_.end()) {
      role_it->second &= ~perm_int;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error removing role permission: " << e.what() << std::endl;
  }
}

void AccessControl::setUserRole(const std::string &username,
                                const std::string &role) {
  user_roles_[username] = role;
}

std::string AccessControl::getUserRole(const std::string &username) const {
  auto user_it = user_roles_.find(username);
  return (user_it != user_roles_.end()) ? user_it->second : "";
}

bool AccessControl::isPathAccessible(const std::string &path,
                                     const std::string &username) const {
  try {
    std::string user_role = getUserRole(username);
    if (user_role.empty()) {
      return false;
    }

    // Admin role has access to everything
    if (user_role == "admin") {
      return true;
    }

    // Check path restrictions
    for (const auto &[pattern, allowed_roles] : path_restrictions_) {
      if (matchesPattern(path, pattern)) {
        return std::find(allowed_roles.begin(), allowed_roles.end(),
                         user_role) != allowed_roles.end();
      }
    }

    // Default: allow access if no specific restrictions
    return true;

  } catch (const std::exception &) {
    return false;
  }
}

void AccessControl::addPathRestriction(
    const std::string &path, const std::vector<std::string> &allowed_roles) {
  path_restrictions_[path] = allowed_roles;
}

void AccessControl::setupDefaultConfiguration() {
  // Default role permissions
  role_permissions_["admin"] = static_cast<int>(Permission::READ) |
                               static_cast<int>(Permission::WRITE) |
                               static_cast<int>(Permission::DELETE) |
                               static_cast<int>(Permission::ADMIN);

  role_permissions_["user"] =
      static_cast<int>(Permission::READ) | static_cast<int>(Permission::WRITE);

  role_permissions_["guest"] = static_cast<int>(Permission::READ);

  // Default user roles
  user_roles_["admin"] = "admin";
  user_roles_["user"] = "user";

  // Default path restrictions
  path_restrictions_["/admin/*"] = {"admin"};
  path_restrictions_["/private/*"] = {"admin", "user"};
}

bool AccessControl::loadConfigFromFile(const std::string &config_path) {
  try {
    std::ifstream file(config_path);
    if (!file.is_open()) {
      // Create default configuration
      setupDefaultConfiguration();
      return true;
    }

    nlohmann::json config;
    file >> config;

    // Load role permissions
    if (config.contains("roles")) {
      for (const auto &[role, perms] : config["roles"].items()) {
        int permission_mask = 0;
        for (const std::string &perm : perms) {
          if (perm == "read")
            permission_mask |= static_cast<int>(Permission::READ);
          else if (perm == "write")
            permission_mask |= static_cast<int>(Permission::WRITE);
          else if (perm == "delete")
            permission_mask |= static_cast<int>(Permission::DELETE);
          else if (perm == "admin")
            permission_mask |= static_cast<int>(Permission::ADMIN);
        }
        role_permissions_[role] = permission_mask;
      }
    }

    // Load user roles
    if (config.contains("users")) {
      for (const auto &[username, role] : config["users"].items()) {
        user_roles_[username] = role;
      }
    }

    // Load path restrictions
    if (config.contains("path_restrictions")) {
      for (const auto &[path, roles] : config["path_restrictions"].items()) {
        std::vector<std::string> allowed_roles = roles;
        path_restrictions_[path] = allowed_roles;
      }
    }

    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error loading access control config: " << e.what()
              << std::endl;
    setupDefaultConfiguration();
    return false;
  }
}

bool AccessControl::matchesPattern(const std::string &path,
                                   const std::string &pattern) const {
  try {
    // Convert glob pattern to regex
    std::string regex_pattern = pattern;

    // Escape special regex characters except * and ?
    std::regex special_chars{R"([-[\]{}()+.,\^$|#\s])"};
    regex_pattern = std::regex_replace(regex_pattern, special_chars, R"(\$&)");

    // Convert glob wildcards to regex
    std::regex glob_star{R"(\\\*)"};
    regex_pattern = std::regex_replace(regex_pattern, glob_star, ".*");

    std::regex glob_question{R"(\\\?)"};
    regex_pattern = std::regex_replace(regex_pattern, glob_question, ".");

    // Add anchors
    regex_pattern = "^" + regex_pattern + "$";

    std::regex pattern_regex(regex_pattern);
    return std::regex_match(path, pattern_regex);

  } catch (const std::exception &) {
    // Fallback to simple string comparison
    return path == pattern;
  }
}

int AccessControl::permissionToInt(Permission permission) const {
  return static_cast<int>(permission);
}

} // namespace security
} // namespace fileserver
