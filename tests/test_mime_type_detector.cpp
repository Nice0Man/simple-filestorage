#include <gtest/gtest.h>
#include "utils/mime_type_detector.h"

using namespace fileserver::utils;

class MimeTypeDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Nothing to set up - MimeTypeDetector is static
    }
};

// Test text file MIME types
TEST_F(MimeTypeDetectorTest, TextFiles) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("file.txt"), "text/plain");
    EXPECT_EQ(MimeTypeDetector::getMimeType("file.html"), "text/html");
    EXPECT_EQ(MimeTypeDetector::getMimeType("file.css"), "text/css");
    EXPECT_EQ(MimeTypeDetector::getMimeType("file.js"), "application/javascript");
}

// Test image file MIME types
TEST_F(MimeTypeDetectorTest, ImageFiles) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("image.jpg"), "image/jpeg");
    EXPECT_EQ(MimeTypeDetector::getMimeType("image.jpeg"), "image/jpeg");
    EXPECT_EQ(MimeTypeDetector::getMimeType("image.png"), "image/png");
    EXPECT_EQ(MimeTypeDetector::getMimeType("image.gif"), "image/gif");
    EXPECT_EQ(MimeTypeDetector::getMimeType("image.svg"), "image/svg+xml");
}

// Test document file MIME types
TEST_F(MimeTypeDetectorTest, DocumentFiles) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("document.pdf"), "application/pdf");
    EXPECT_EQ(MimeTypeDetector::getMimeType("document.doc"), "application/msword");
    EXPECT_EQ(MimeTypeDetector::getMimeType("document.docx"), 
              "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
}

// Test archive file MIME types
TEST_F(MimeTypeDetectorTest, ArchiveFiles) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("archive.zip"), "application/zip");
    EXPECT_EQ(MimeTypeDetector::getMimeType("archive.tar"), "application/x-tar");
    EXPECT_EQ(MimeTypeDetector::getMimeType("archive.gz"), "application/gzip");
}

// Test MIME type detection by extension
TEST_F(MimeTypeDetectorTest, MimeTypeByExtension) {
    EXPECT_EQ(MimeTypeDetector::getMimeTypeByExtension(".txt"), "text/plain");
    EXPECT_EQ(MimeTypeDetector::getMimeTypeByExtension("txt"), "text/plain");
    EXPECT_EQ(MimeTypeDetector::getMimeTypeByExtension(".PDF"), "application/pdf");
}

// Test unknown file types
TEST_F(MimeTypeDetectorTest, UnknownFileType) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("file.unknown"), "application/octet-stream");
    EXPECT_EQ(MimeTypeDetector::getMimeType("file"), "application/octet-stream");
}

// Test case insensitivity
TEST_F(MimeTypeDetectorTest, CaseInsensitive) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("FILE.TXT"), "text/plain");
    EXPECT_EQ(MimeTypeDetector::getMimeType("FILE.PDF"), "application/pdf");
    EXPECT_EQ(MimeTypeDetector::getMimeType("FILE.JpG"), "image/jpeg");
}

// Test allowed file types
TEST_F(MimeTypeDetectorTest, AllowedFileTypes) {
    // Common allowed types
    EXPECT_TRUE(MimeTypeDetector::isAllowedFileType("document.txt"));
    EXPECT_TRUE(MimeTypeDetector::isAllowedFileType("image.jpg"));
    EXPECT_TRUE(MimeTypeDetector::isAllowedFileType("document.pdf"));
    EXPECT_TRUE(MimeTypeDetector::isAllowedFileType("document.doc"));
    EXPECT_TRUE(MimeTypeDetector::isAllowedFileType("document.docx"));
}

// Test custom MIME type mapping
TEST_F(MimeTypeDetectorTest, CustomMimeTypeMapping) {
    MimeTypeDetector::addMimeTypeMapping(".custom", "application/x-custom");
    
    EXPECT_EQ(MimeTypeDetector::getMimeType("file.custom"), "application/x-custom");
}

// Test files with multiple dots
TEST_F(MimeTypeDetectorTest, MultipleDotsInFilename) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("my.file.name.txt"), "text/plain");
    EXPECT_EQ(MimeTypeDetector::getMimeType("archive.tar.gz"), "application/gzip");
}

// Test files with path
TEST_F(MimeTypeDetectorTest, FilesWithPath) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("/path/to/file.txt"), "text/plain");
    EXPECT_EQ(MimeTypeDetector::getMimeType("relative/path/file.pdf"), "application/pdf");
}

// Test XML and JSON files
TEST_F(MimeTypeDetectorTest, DataFiles) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("data.json"), "application/json");
    EXPECT_EQ(MimeTypeDetector::getMimeType("data.xml"), "application/xml");
}

// Test video files
TEST_F(MimeTypeDetectorTest, VideoFiles) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("video.mp4"), "video/mp4");
    EXPECT_EQ(MimeTypeDetector::getMimeType("video.avi"), "video/x-msvideo");
}

// Test audio files
TEST_F(MimeTypeDetectorTest, AudioFiles) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("audio.mp3"), "audio/mpeg");
    EXPECT_EQ(MimeTypeDetector::getMimeType("audio.wav"), "audio/wav");
}

// Test empty filename
TEST_F(MimeTypeDetectorTest, EmptyFilename) {
    EXPECT_EQ(MimeTypeDetector::getMimeType(""), "application/octet-stream");
}

// Test filename without extension
TEST_F(MimeTypeDetectorTest, NoExtension) {
    EXPECT_EQ(MimeTypeDetector::getMimeType("README"), "application/octet-stream");
    EXPECT_EQ(MimeTypeDetector::getMimeType("Makefile"), "application/octet-stream");
}

