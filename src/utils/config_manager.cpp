#include "utils/config_manager.h"
#include <fstream>
#include <iostream>

namespace fileserver {
namespace utils {

bool ConfigManager::loadFromFile(const std::string& config_path) {
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "Cannot open config file: " << config_path << std::endl;
            return false;
        }
        
        file >> config_data_;
        return true;
        
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Config loading error: " << e.what() << std::endl;
        return false;
    }
}

std::string ConfigManager::getString(const std::string& key, const std::string& default_value) const {
    return getValue(key, default_value);
}

int ConfigManager::getInt(const std::string& key, int default_value) const {
    return getValue(key, default_value);
}

bool ConfigManager::getBool(const std::string& key, bool default_value) const {
    return getValue(key, default_value);
}

double ConfigManager::getDouble(const std::string& key, double default_value) const {
    return getValue(key, default_value);
}

bool ConfigManager::hasKey(const std::string& key) const {
    try {
        // Support nested keys with dot notation (e.g., "server.port")
        auto keys = splitKey(key);
        nlohmann::json current = config_data_;
        
        for (const auto& k : keys) {
            if (!current.contains(k)) {
                return false;
            }
            current = current[k];
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void ConfigManager::set(const std::string& key, const std::string& value) {
    try {
        auto keys = splitKey(key);
        nlohmann::json* current = &config_data_;
        
        // Navigate to the parent of the target key
        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (!current->contains(keys[i])) {
                (*current)[keys[i]] = nlohmann::json::object();
            }
            current = &(*current)[keys[i]];
        }
        
        // Set the value
        (*current)[keys.back()] = value;
        
    } catch (const std::exception& e) {
        std::cerr << "Error setting config value: " << e.what() << std::endl;
    }
}

bool ConfigManager::saveToFile(const std::string& config_path) const {
    try {
        std::ofstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "Cannot open config file for writing: " << config_path << std::endl;
            return false;
        }
        
        file << config_data_.dump(4); // Pretty print with 4 spaces
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Config saving error: " << e.what() << std::endl;
        return false;
    }
}

template<typename T>
T ConfigManager::getValue(const std::string& key, const T& default_value) const {
    try {
        auto keys = splitKey(key);
        nlohmann::json current = config_data_;
        
        for (const auto& k : keys) {
            if (!current.contains(k)) {
                return default_value;
            }
            current = current[k];
        }
        
        return current.get<T>();
        
    } catch (const std::exception&) {
        return default_value;
    }
}

std::vector<std::string> ConfigManager::splitKey(const std::string& key) const {
    std::vector<std::string> keys;
    std::string current_key;
    
    for (char c : key) {
        if (c == '.') {
            if (!current_key.empty()) {
                keys.push_back(current_key);
                current_key.clear();
            }
        } else {
            current_key += c;
        }
    }
    
    if (!current_key.empty()) {
        keys.push_back(current_key);
    }
    
    return keys;
}

// Explicit template instantiations
template std::string ConfigManager::getValue<std::string>(const std::string&, const std::string&) const;
template int ConfigManager::getValue<int>(const std::string&, const int&) const;
template bool ConfigManager::getValue<bool>(const std::string&, const bool&) const;
template double ConfigManager::getValue<double>(const std::string&, const double&) const;

} // namespace utils
} // namespace fileserver
