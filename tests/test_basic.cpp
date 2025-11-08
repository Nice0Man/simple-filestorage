#include <gtest/gtest.h>

// Basic test to ensure the test framework is working
TEST(BasicTest, AlwaysPass) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}

TEST(BasicTest, StringTest) {
    std::string hello = "Hello";
    std::string world = "World";
    EXPECT_NE(hello, world);
    EXPECT_EQ(hello + " " + world, "Hello World");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
