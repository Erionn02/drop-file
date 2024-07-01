#include <gtest/gtest.h>

#include "FileHash.hpp"

#include <spdlog/spdlog.h>

#include <fstream>
#include <random>

using namespace ::testing;

struct FileHashTests : public Test {
    std::filesystem::path file_1_path{std::filesystem::temp_directory_path() / "file1"};
    std::filesystem::path file_2_path{std::filesystem::temp_directory_path() / "file2"};

    void TearDown() override {
        std::filesystem::remove(file_1_path);
        std::filesystem::remove(file_2_path);
    }

    std::string generateRandomString(std::size_t length) {
        static auto &chrs = "0123456789"
                            "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        static thread_local std::mt19937 rg{std::random_device{}()};
        static thread_local std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

        std::string random_str;
        random_str.reserve(length);
        while (length--) {
            random_str += chrs[pick(rg)];
        }

        return random_str;
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