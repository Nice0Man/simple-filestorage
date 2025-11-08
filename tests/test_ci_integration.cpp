#include <gtest/gtest.h>
#include <string>

// Simple integration test to verify CI/CD pipeline
class CIIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }

    void TearDown() override {
        // Cleanup code
    }
};

TEST_F(CIIntegrationTest, BasicFunctionality) {
    EXPECT_TRUE(true);
    EXPECT_EQ(1 + 1, 2);
}

TEST_F(CIIntegrationTest, StringOperations) {
    std::string test = "CI/CD Pipeline";
    EXPECT_FALSE(test.empty());
    EXPECT_EQ(test.length(), 14);
}

TEST_F(CIIntegrationTest, MathOperations) {
    int a = 10;
    int b = 20;
    EXPECT_EQ(a + b, 30);
    EXPECT_GT(b, a);
    EXPECT_LT(a, b);
}

// Test that will always pass - for CI validation
TEST(BasicTest, AlwaysPass) {
    EXPECT_TRUE(true);
}

// Test environment setup
TEST(EnvironmentTest, CompilerCheck) {
#ifdef __GNUC__
    EXPECT_TRUE(true) << "GCC compiler detected";
#endif

#ifdef __clang__
    EXPECT_TRUE(true) << "Clang compiler detected";
#endif

#ifdef _MSC_VER
    EXPECT_TRUE(true) << "MSVC compiler detected";
#endif
}

// C++17 feature test
TEST(CPP17Test, StructuredBindings) {
    auto pair = std::make_pair(1, 2);
    auto [first, second] = pair;
    EXPECT_EQ(first, 1);
    EXPECT_EQ(second, 2);
}

