#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace fileserver {
namespace utils {

/**
 * @brief Configuration manager class
 * 
 * Manages server configuration from JSON files
 */
class ConfigManager {
public:
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    /**
     * @brief Load configuration from file
     * @param config_path Path to configuration file
     * @return true if loading successful
     */
    bool loadFromFile(const std::string& config_path);
    
    /**
     * @brief Get string configuration value
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return Configuration value
     */
    std::string getString(const std::string& key, const std::string& default_value = "") const;
    
    /**
     * @brief Get integer configuration value
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return Configuration value
     */
    int getInt(const std::string& key, int default_value = 0) const;
    
    /**
     * @brief Get boolean configuration value
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return Configuration value
     */
    bool getBool(const std::string& key, bool default_value = false) const;
    
    /**
     * @brief Get double configuration value
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return Configuration value
     */
    double getDouble(const std::string& key, double default_value = 0.0) const;
    
    /**
     * @brief Check if key exists
     * @param key Configuration key
     * @return true if key exists
     */
    bool hasKey(const std::string& key) const;
    
    /**
     * @brief Set configuration value
     * @param key Configuration key
     * @param value Configuration value
     */
    void set(const std::string& key, const std::string& value);
    
    /**
     * @brief Save configuration to file
     * @param config_path Path to save configuration
     * @return true if saving successful
     */
    bool saveToFile(const std::string& config_path) const;

private:
    nlohmann::json config_data_;
    
    template<typename T>
    T getValue(const std::string& key, const T& default_value) const;
    
    std::vector<std::string> splitKey(const std::string& key) const;
};

} // namespace utils
} // namespace fileserver