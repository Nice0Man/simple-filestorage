#include <gtest/gtest.h>
#include "security/auth_manager.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;
using namespace fileserver::security;

class AuthManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test files
        test_dir_ = fs::temp_directory_path() / "auth_test";
        fs::create_directories(test_dir_);
        users_file_path_ = test_dir_ / "users.json";
        
        // Create test users file
        createTestUsersFile();
        
        auth_manager_ = std::make_unique<AuthManager>();
        auth_manager_->initialize(users_file_path_.string());
    }
    
    void TearDown() override {
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }
    
    void createTestUsersFile() {
        std::string users_json = R"({
            "users": [
                {
                    "username": "admin",
                    "password_hash": "240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9",
                    "salt": "",
                    "role": "admin",
                    "email": "admin@test.com"
                },
                {
                    "username": "user",
                    "password_hash": "e606e38b0d8c19b24cf0ee3808183162ea7cd63ff7912dbb22b5e803286b4446",
                    "salt": "",
                    "role": "user",
                    "email": "user@test.com"
                }
            ]
        })";
        
        std::ofstream file(users_file_path_);
        file << users_json;
        file.close();
    }
    
    fs::path test_dir_;
    fs::path users_file_path_;
    std::unique_ptr<AuthManager> auth_manager_;
};

// Test AuthManager construction
TEST_F(AuthManagerTest, Construction) {
    ASSERT_NE(auth_manager_, nullptr);
}

// Test successful authentication with admin user
TEST_F(AuthManagerTest, AuthenticateAdminSuccess) {
    std::string token = auth_manager_->authenticate("admin", "admin123");
    EXPECT_FALSE(token.empty());
}

// Test successful authentication with regular user
TEST_F(AuthManagerTest, AuthenticateUserSuccess) {
    std::string token = auth_manager_->authenticate("user", "user123");
    EXPECT_FALSE(token.empty());
}

// Test failed authentication with wrong password
TEST_F(AuthManagerTest, AuthenticateWrongPassword) {
    std::string token = auth_manager_->authenticate("admin", "wrongpassword");
    EXPECT_TRUE(token.empty());
}

// Test failed authentication with non-existent user
TEST_F(AuthManagerTest, AuthenticateNonExistentUser) {
    std::string token = auth_manager_->authenticate("nonexistent", "password");
    EXPECT_TRUE(token.empty());
}

// Test token validation
TEST_F(AuthManagerTest, ValidateToken) {
    std::string token = auth_manager_->authenticate("admin", "admin123");
    ASSERT_FALSE(token.empty());
    
    bool valid = auth_manager_->validateToken(token);
    EXPECT_TRUE(valid);
}

// Test invalid token validation
TEST_F(AuthManagerTest, ValidateInvalidToken) {
    bool valid = auth_manager_->validateToken("invalid_token_string");
    EXPECT_FALSE(valid);
}

// Test get user by token
TEST_F(AuthManagerTest, GetUserByToken) {
    std::string token = auth_manager_->authenticate("admin", "admin123");
    ASSERT_FALSE(token.empty());
    
    auto user = auth_manager_->getUserByToken(token);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->username, "admin");
    EXPECT_EQ(user->role, "admin");
}

// Test get user by invalid token
TEST_F(AuthManagerTest, GetUserByInvalidToken) {
    auto user = auth_manager_->getUserByToken("invalid_token");
    EXPECT_EQ(user, nullptr);
}

// Test token revocation
TEST_F(AuthManagerTest, RevokeToken) {
    std::string token = auth_manager_->authenticate("admin", "admin123");
    ASSERT_FALSE(token.empty());
    
    bool revoked = auth_manager_->revokeToken(token);
    EXPECT_TRUE(revoked);
    
    // Token should no longer be valid
    bool valid = auth_manager_->validateToken(token);
    EXPECT_FALSE(valid);
}

// Test adding new user
TEST_F(AuthManagerTest, AddUser) {
    bool added = auth_manager_->addUser("newuser", "newpassword", "user");
    EXPECT_TRUE(added);
    
    // Should be able to authenticate with new user
    std::string token = auth_manager_->authenticate("newuser", "newpassword");
    EXPECT_FALSE(token.empty());
}

// Test adding duplicate user
TEST_F(AuthManagerTest, AddDuplicateUser) {
    bool added = auth_manager_->addUser("admin", "password", "user");
    EXPECT_FALSE(added);
}

// Test removing user
TEST_F(AuthManagerTest, RemoveUser) {
    bool removed = auth_manager_->removeUser("user");
    EXPECT_TRUE(removed);
    
    // Should not be able to authenticate after removal
    std::string token = auth_manager_->authenticate("user", "user123");
    EXPECT_TRUE(token.empty());
}

// Test removing non-existent user
TEST_F(AuthManagerTest, RemoveNonExistentUser) {
    bool removed = auth_manager_->removeUser("nonexistent");
    EXPECT_FALSE(removed);
}

// Test changing password with correct old password
TEST_F(AuthManagerTest, ChangePasswordSuccess) {
    bool changed = auth_manager_->changePassword("user", "user123", "newpassword123");
    EXPECT_TRUE(changed);
    
    // Should authenticate with new password
    std::string token = auth_manager_->authenticate("user", "newpassword123");
    EXPECT_FALSE(token.empty());
    
    // Should not authenticate with old password
    std::string old_token = auth_manager_->authenticate("user", "user123");
    EXPECT_TRUE(old_token.empty());
}

// Test changing password with wrong old password
TEST_F(AuthManagerTest, ChangePasswordWrongOldPassword) {
    bool changed = auth_manager_->changePassword("user", "wrongpassword", "newpassword123");
    EXPECT_FALSE(changed);
}

// Test changing password for non-existent user
TEST_F(AuthManagerTest, ChangePasswordNonExistentUser) {
    bool changed = auth_manager_->changePassword("nonexistent", "old", "new");
    EXPECT_FALSE(changed);
}

// Test multiple authentication attempts
TEST_F(AuthManagerTest, MultipleAuthentications) {
    std::string token1 = auth_manager_->authenticate("admin", "admin123");
    std::string token2 = auth_manager_->authenticate("admin", "admin123");
    std::string token3 = auth_manager_->authenticate("user", "user123");
    
    EXPECT_FALSE(token1.empty());
    EXPECT_FALSE(token2.empty());
    EXPECT_FALSE(token3.empty());
    
    // All tokens should be different
    EXPECT_NE(token1, token2);
    EXPECT_NE(token1, token3);
    EXPECT_NE(token2, token3);
    
    // All tokens should be valid
    EXPECT_TRUE(auth_manager_->validateToken(token1));
    EXPECT_TRUE(auth_manager_->validateToken(token2));
    EXPECT_TRUE(auth_manager_->validateToken(token3));
}

// Test token persistence across multiple operations
TEST_F(AuthManagerTest, TokenPersistence) {
    std::string token = auth_manager_->authenticate("admin", "admin123");
    ASSERT_FALSE(token.empty());
    
    // Perform multiple validations
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(auth_manager_->validateToken(token));
    }
    
    // Get user info multiple times
    for (int i = 0; i < 10; ++i) {
        auto user = auth_manager_->getUserByToken(token);
        ASSERT_NE(user, nullptr);
        EXPECT_EQ(user->username, "admin");
    }
}

// Test empty credentials
TEST_F(AuthManagerTest, EmptyCredentials) {
    std::string token1 = auth_manager_->authenticate("", "");
    std::string token2 = auth_manager_->authenticate("admin", "");
    std::string token3 = auth_manager_->authenticate("", "password");
    
    EXPECT_TRUE(token1.empty());
    EXPECT_TRUE(token2.empty());
    EXPECT_TRUE(token3.empty());
}

// Test clean expired tokens (basic test)
TEST_F(AuthManagerTest, CleanExpiredTokens) {
    // This is a basic test - in a real scenario, we would need to wait for tokens to expire
    // or manipulate time to test expired token cleanup
    
    std::string token = auth_manager_->authenticate("admin", "admin123");
    ASSERT_FALSE(token.empty());
    
    // Clean expired tokens
    auth_manager_->cleanExpiredTokens();
    
    // Token should still be valid (not expired yet)
    EXPECT_TRUE(auth_manager_->validateToken(token));
}

