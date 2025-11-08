#include <gtest/gtest.h>
#include "core/file_manager.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace fileserver::core;

class FileManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test directory
        test_dir_ = fs::temp_directory_path() / "fileserver_test";
        fs::create_directories(test_dir_);
        
        file_manager_ = std::make_unique<FileManager>(test_dir_.string());
    }
    
    void TearDown() override {
        // Clean up test directory
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }
    
    void createTestFile(const std::string& name, const std::string& content) {
        std::ofstream file(test_dir_ / name);
        file << content;
        file.close();
    }
    
    fs::path test_dir_;
    std::unique_ptr<FileManager> file_manager_;
};

// Test FileManager construction
TEST_F(FileManagerTest, Construction) {
    ASSERT_NE(file_manager_, nullptr);
}

// Test file existence check
TEST_F(FileManagerTest, FileExists) {
    createTestFile("test.txt", "test content");
    
    EXPECT_TRUE(file_manager_->fileExists("test.txt"));
    EXPECT_FALSE(file_manager_->fileExists("nonexistent.txt"));
}

// Test file listing
TEST_F(FileManagerTest, ListFiles) {
    createTestFile("file1.txt", "content1");
    createTestFile("file2.txt", "content2");
    createTestFile("file3.pdf", "content3");
    
    auto files = file_manager_->listFiles();
    
    EXPECT_EQ(files.size(), 3);
    
    // Check that all files are found
    std::vector<std::string> file_names;
    for (const auto& file : files) {
        file_names.push_back(file.name);
    }
    
    EXPECT_TRUE(std::find(file_names.begin(), file_names.end(), "file1.txt") != file_names.end());
    EXPECT_TRUE(std::find(file_names.begin(), file_names.end(), "file2.txt") != file_names.end());
    EXPECT_TRUE(std::find(file_names.begin(), file_names.end(), "file3.pdf") != file_names.end());
}

// Test file size retrieval
TEST_F(FileManagerTest, GetFileSize) {
    std::string content = "This is test content";
    createTestFile("sized_file.txt", content);
    
    auto size = file_manager_->getFileSize("sized_file.txt");
    EXPECT_EQ(size, content.length());
}

// Test path safety validation
TEST_F(FileManagerTest, IsPathSafe) {
    EXPECT_TRUE(file_manager_->isPathSafe("safe_file.txt"));
    EXPECT_TRUE(file_manager_->isPathSafe("subdir/file.txt"));
    EXPECT_FALSE(file_manager_->isPathSafe("../etc/passwd"));
    EXPECT_FALSE(file_manager_->isPathSafe("/absolute/path"));
}

// Test upload strategy
TEST_F(FileManagerTest, UploadStrategy) {
    auto upload_strategy = std::make_unique<UploadStrategy>();
    file_manager_->setStrategy(std::move(upload_strategy));
    
    std::string test_content = "Upload test content";
    bool result = file_manager_->executeOperation("uploaded_file.txt", test_content);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(file_manager_->fileExists("uploaded_file.txt"));
}

// Test delete strategy
TEST_F(FileManagerTest, DeleteStrategy) {
    createTestFile("to_delete.txt", "content");
    
    EXPECT_TRUE(file_manager_->fileExists("to_delete.txt"));
    
    auto delete_strategy = std::make_unique<DeleteStrategy>();
    file_manager_->setStrategy(std::move(delete_strategy));
    
    bool result = file_manager_->executeOperation("to_delete.txt");
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(file_manager_->fileExists("to_delete.txt"));
}

// Test download strategy
TEST_F(FileManagerTest, DownloadStrategy) {
    std::string content = "Download test content";
    createTestFile("to_download.txt", content);
    
    auto download_strategy = std::make_unique<DownloadStrategy>();
    file_manager_->setStrategy(std::move(download_strategy));
    
    bool result = file_manager_->executeOperation("to_download.txt");
    
    EXPECT_TRUE(result);
    std::string downloaded_content = file_manager_->getOperationResult();
    EXPECT_EQ(downloaded_content, content);
}

// Test subdirectory handling
TEST_F(FileManagerTest, SubdirectoryHandling) {
    fs::create_directories(test_dir_ / "subdir");
    createTestFile("subdir/nested_file.txt", "nested content");
    
    EXPECT_TRUE(file_manager_->fileExists("subdir/nested_file.txt"));
    EXPECT_TRUE(file_manager_->isPathSafe("subdir/nested_file.txt"));
}

// Test empty directory listing
TEST_F(FileManagerTest, EmptyDirectoryListing) {
    auto files = file_manager_->listFiles();
    EXPECT_EQ(files.size(), 0);
}

// Test large file size
TEST_F(FileManagerTest, LargeFileSize) {
    std::string large_content(10000, 'x');
    createTestFile("large_file.txt", large_content);
    
    auto size = file_manager_->getFileSize("large_file.txt");
    EXPECT_EQ(size, 10000);
}

