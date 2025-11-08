#include <gtest/gtest.h>
#include "utils/config_manager.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace fileserver::utils;

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_unique<ConfigManager>();
        
        // Create temporary directory for test configs
        test_dir_ = fs::temp_directory_path() / "config_test";
        fs::create_directories(test_dir_);
        test_config_path_ = test_dir_ / "test_config.json";
    }
    
    void TearDown() override {
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }
    
    void createTestConfig(const std::string& json_content) {
        std::ofstream file(test_config_path_);
        file << json_content;
        file.close();
    }
    
    std::unique_ptr<ConfigManager> config_manager_;
    fs::path test_dir_;
    fs::path test_config_path_;
};

// Test ConfigManager construction
TEST_F(ConfigManagerTest, Construction) {
    ASSERT_NE(config_manager_, nullptr);
}

// Test loading valid JSON configuration
TEST_F(ConfigManagerTest, LoadValidConfig) {
    std::string config_json = R"({
        "server": {
            "host": "localhost",
            "port": 8080
        },
        "database": {
            "name": "testdb"
        }
    })";
    
    createTestConfig(config_json);
    
    bool result = config_manager_->loadFromFile(test_config_path_.string());
    EXPECT_TRUE(result);
}

// Test getString method
TEST_F(ConfigManagerTest, GetString) {
    std::string config_json = R"({
        "server": {
            "host": "localhost"
        }
    })";
    
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    std::string host = config_manager_->getString("server.host", "default");
    EXPECT_EQ(host, "localhost");
}

// Test getString with default value
TEST_F(ConfigManagerTest, GetStringDefault) {
    std::string config_json = "{}";
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    std::string value = config_manager_->getString("nonexistent.key", "default_value");
    EXPECT_EQ(value, "default_value");
}

// Test getInt method
TEST_F(ConfigManagerTest, GetInt) {
    std::string config_json = R"({
        "server": {
            "port": 8080
        }
    })";
    
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    int port = config_manager_->getInt("server.port", 0);
    EXPECT_EQ(port, 8080);
}

// Test getInt with default value
TEST_F(ConfigManagerTest, GetIntDefault) {
    std::string config_json = "{}";
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    int value = config_manager_->getInt("nonexistent.key", 12345);
    EXPECT_EQ(value, 12345);
}

// Test getBool method
TEST_F(ConfigManagerTest, GetBool) {
    std::string config_json = R"({
        "features": {
            "enable_logging": true,
            "enable_debug": false
        }
    })";
    
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    bool logging = config_manager_->getBool("features.enable_logging", false);
    bool debug = config_manager_->getBool("features.enable_debug", true);
    
    EXPECT_TRUE(logging);
    EXPECT_FALSE(debug);
}

// Test getBool with default value
TEST_F(ConfigManagerTest, GetBoolDefault) {
    std::string config_json = "{}";
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    bool value = config_manager_->getBool("nonexistent.key", true);
    EXPECT_TRUE(value);
}

// Test getDouble method
TEST_F(ConfigManagerTest, GetDouble) {
    std::string config_json = R"({
        "performance": {
            "timeout": 30.5
        }
    })";
    
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    double timeout = config_manager_->getDouble("performance.timeout", 0.0);
    EXPECT_DOUBLE_EQ(timeout, 30.5);
}

// Test getDouble with default value
TEST_F(ConfigManagerTest, GetDoubleDefault) {
    std::string config_json = "{}";
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    double value = config_manager_->getDouble("nonexistent.key", 123.456);
    EXPECT_DOUBLE_EQ(value, 123.456);
}

// Test hasKey method
TEST_F(ConfigManagerTest, HasKey) {
    std::string config_json = R"({
        "server": {
            "host": "localhost"
        }
    })";
    
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    EXPECT_TRUE(config_manager_->hasKey("server.host"));
    EXPECT_FALSE(config_manager_->hasKey("server.port"));
}

// Test set method
TEST_F(ConfigManagerTest, SetValue) {
    config_manager_->set("new.key", "new_value");
    
    std::string value = config_manager_->getString("new.key", "");
    EXPECT_EQ(value, "new_value");
}

// Test saveToFile method
TEST_F(ConfigManagerTest, SaveToFile) {
    config_manager_->set("test.key", "test_value");
    
    fs::path save_path = test_dir_ / "saved_config.json";
    bool result = config_manager_->saveToFile(save_path.string());
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(save_path));
}

// Test loading invalid JSON
TEST_F(ConfigManagerTest, LoadInvalidJSON) {
    createTestConfig("{invalid json}");
    
    bool result = config_manager_->loadFromFile(test_config_path_.string());
    EXPECT_FALSE(result);
}

// Test loading non-existent file
TEST_F(ConfigManagerTest, LoadNonExistentFile) {
    bool result = config_manager_->loadFromFile("/nonexistent/path/config.json");
    EXPECT_FALSE(result);
}

// Test nested key access
TEST_F(ConfigManagerTest, NestedKeyAccess) {
    std::string config_json = R"({
        "level1": {
            "level2": {
                "level3": {
                    "value": "deep_value"
                }
            }
        }
    })";
    
    createTestConfig(config_json);
    config_manager_->loadFromFile(test_config_path_.string());
    
    std::string value = config_manager_->getString("level1.level2.level3.value", "");
    EXPECT_EQ(value, "deep_value");
}

// Test empty configuration
TEST_F(ConfigManagerTest, EmptyConfiguration) {
    createTestConfig("{}");
    config_manager_->loadFromFile(test_config_path_.string());
    
    EXPECT_FALSE(config_manager_->hasKey("any.key"));
}

