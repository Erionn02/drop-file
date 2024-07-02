#include <gtest/gtest.h>

#include "FileHash.hpp"
#include "Utils.hpp"

#include <spdlog/spdlog.h>

#include <fstream>

using namespace ::testing;

struct FileHashTests : public Test {
    std::filesystem::path file_1_path{std::filesystem::temp_directory_path() / "file1"};
    std::filesystem::path file_2_path{std::filesystem::temp_directory_path() / "file2"};

    void TearDown() override {
        std::filesystem::remove(file_1_path);
        std::filesystem::remove(file_2_path);
    }
};

TEST_F(FileHashTests, throwsOnNonExistentFile) {
    ASSERT_THROW(calculateFileHash(file_1_path), std::runtime_error);
}

TEST_F(FileHashTests, sameFileContentOfDifferentFilesResultInSameHash) {
    std::string content{"Hello world, wassup"};
    std::ofstream file1{file_1_path, std::ios::trunc};
    std::ofstream file2{file_2_path, std::ios::trunc};
    file1 << content;
    file2 << content;
    ASSERT_EQ(calculateFileHash(file_1_path), calculateFileHash(file_2_path));
}

TEST_F(FileHashTests, hashingTheSameFileTwiceResultsInTheSameHash) {
    std::string content{"Hello world, some content"};
    std::ofstream file1{file_1_path, std::ios::trunc};
    file1 << content;
    ASSERT_EQ(calculateFileHash(file_1_path), calculateFileHash(file_1_path));
}

TEST_F(FileHashTests, doesNotCrashOnCalculatingHashOfBigFile) {
    std::ofstream file1{file_1_path, std::ios::trunc};
    std::size_t content_length{8 * 1 << 20}; // 8 MB
    file1 << generateRandomString(content_length);

    auto hash = calculateFileHash(file_1_path);
    spdlog::info(hash);
    ASSERT_NE(hash, "");
}