#include <gtest/gtest.h>

#include "Utils.hpp"
#include "TestHelpers.hpp"

#include <spdlog/spdlog.h>

#include <fstream>

using namespace ::testing;

struct UtilsTests : public Test {
    std::filesystem::path file_1_path{std::filesystem::temp_directory_path() / "file1"};
    std::filesystem::path file_2_path{std::filesystem::temp_directory_path() / "file2"};
    const std::size_t SINGLE_DIR_SIZE{4096};

    void TearDown() override {
        std::filesystem::remove_all(file_1_path);
        std::filesystem::remove_all(file_2_path);
    }
};

TEST_F(UtilsTests, calculateFileHashThrowsOnNonExistentFile) {
    ASSERT_THROW(calculateFileHash(file_1_path), UtilsException);
}

TEST_F(UtilsTests, sameFileContentOfDifferentFilesResultInSameHash) {
    std::string content{"Hello world, wassup"};
    std::ofstream file1{file_1_path, std::ios::trunc};
    std::ofstream file2{file_2_path, std::ios::trunc};
    file1 << content;
    file2 << content;
    ASSERT_EQ(calculateFileHash(file_1_path), calculateFileHash(file_2_path));
}

TEST_F(UtilsTests, hashingTheSameFileTwiceResultsInTheSameHash) {
    std::string content{"Hello world, some content"};
    std::ofstream file1{file_1_path, std::ios::trunc};
    file1 << content;
    ASSERT_EQ(calculateFileHash(file_1_path), calculateFileHash(file_1_path));
}

TEST_F(UtilsTests, doesNotCrashOnCalculatingHashOfBigFile) {
    std::ofstream file1{file_1_path, std::ios::trunc};
    std::size_t content_length{8 * 1 << 20}; // 8 MB
    file1 << generateRandomString(content_length);

    auto hash = calculateFileHash(file_1_path);
    spdlog::info(hash);
    ASSERT_NE(hash, "");
}

TEST_F(UtilsTests, bytesToHumanReadableTest) {
    ASSERT_EQ(bytesToHumanReadable(1023), "1023 B");
    ASSERT_EQ(bytesToHumanReadable(1024), "1 kiB");
    ASSERT_EQ(bytesToHumanReadable(1024*1024 -1), "1023 kiB");
    ASSERT_EQ(bytesToHumanReadable(1024*1024), "1 MiB");
    ASSERT_EQ(bytesToHumanReadable(1024*1024*1024-1), "1023 MiB");
    ASSERT_EQ(bytesToHumanReadable(1024*1024*1024), "1 GiB");
    std::size_t number = 2ull*1024ull*1024ull*1024ull;
    ASSERT_EQ(bytesToHumanReadable(number), "2 GiB");
    number = 123ull*1024ull*1024ull*1024ull;
    ASSERT_EQ(bytesToHumanReadable(number), "123 GiB");
}

TEST_F(UtilsTests, getRemainingBytes) {
    std::size_t content_size{10000};
    std::string content{generateRandomString(content_size)};
    {
        std::ofstream file{file_1_path};
        file << content;
    }
    std::ifstream file{file_1_path};


    ASSERT_EQ(getRemainingBytes(file, content_size), content_size);
}

TEST_F(UtilsTests, getRemainingBytesWithOffset) {
    std::size_t content_size{10000};
    std::string content{generateRandomString(content_size)};
    {
        std::ofstream file{file_1_path};
        file << content;
    }
    std::ifstream file{file_1_path};

    std::size_t offset = 100;
    file.seekg(offset);

    ASSERT_EQ(getRemainingBytes(file, content_size), content_size - offset);
}
