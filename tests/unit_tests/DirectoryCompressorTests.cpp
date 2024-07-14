#include <gtest/gtest.h>

#include "DirectoryCompressor.hpp"
#include "TestHelpers.hpp"
#include "Utils.hpp"

#include <spdlog/spdlog.h>


using namespace ::testing;

struct DirectoryCompressorTests: public Test{
    fs::path output_dir_path {fs::temp_directory_path() / "test_dir_output"};
    fs::path input_dir_path {fs::temp_directory_path() / "test_dir_input"};
    fs::path archive_path {fs::temp_directory_path() / "test_archive"};
    
    void SetUp() override {
        deleteTestFiles();
    }
    
    void TearDown() override {
        deleteTestFiles();
    }
    
    void deleteTestFiles() {
        fs::remove_all(output_dir_path);
        fs::remove_all(input_dir_path);
        fs::remove_all(archive_path);
    }

    void setupInputDir() {
        fs::create_directories(input_dir_path);
        {
            std::ofstream file{input_dir_path / "test_file.txt"};
            file << "Some content";
        }
        fs::create_directories(input_dir_path / "nested_dir");
        {
            std::ofstream file{input_dir_path / "nested_dir" / "test_file.txt"};
            file << "Some other content";
        }
        fs::create_directories(input_dir_path / "another_nested_dir");

        std::ofstream empty_file{input_dir_path / "another_nested_dir" / "empty_file"};

        fs::create_directories(input_dir_path / "empty_dir");
    }

    void createLargeFile() {
        spdlog::info("Creating large file...");
        std::ofstream huge_file{input_dir_path / "huge_file.bin", std::ios::binary};
        std::size_t hundred_mb_size{10ull * 1024ull * 1024ull};
        auto str = generateRandomString(hundred_mb_size);
        for(uint i=0; i< 4; ++i) {
            huge_file << str;
        }
        huge_file.flush();
        spdlog::info("Created large file.");
    }

};

TEST_F(DirectoryCompressorTests, cannotCompressToArchiveThatAlreadyExists) {
    setupInputDir();

    DirectoryCompressor dc1{input_dir_path};

    std::ofstream f{archive_path};
    ASSERT_THROW(dc1.createArchive(archive_path), DirectoryCompressorException);
}

TEST_F(DirectoryCompressorTests, cannotDecompressToDirectoryThatAlreadyExists) {
    setupInputDir();

    DirectoryCompressor dc1{input_dir_path};
    ASSERT_THROW(dc1.unpackArchive(archive_path), DirectoryCompressorException);
}

TEST_F(DirectoryCompressorTests, canCompressAndDecompress) {
    setupInputDir();

    DirectoryCompressor dc1{input_dir_path};
    dc1.createArchive(archive_path);

    std::filesystem::create_directories(output_dir_path);
    DirectoryCompressor dc2{output_dir_path};
    dc2.unpackArchive(archive_path);
    
    ASSERT_TRUE(fs::exists(archive_path));
    ASSERT_TRUE(fs::exists(output_dir_path / input_dir_path.filename()));
    assertDirectoriesEqual(input_dir_path, output_dir_path / input_dir_path.filename());
}

TEST_F(DirectoryCompressorTests, canCompressAndDecompressEmptyDir) {
    std::filesystem::create_directories(input_dir_path);

    DirectoryCompressor dc1{input_dir_path};
    dc1.createArchive(archive_path);

    DirectoryCompressor dc2{output_dir_path};
    std::filesystem::create_directories(output_dir_path);
    dc2.unpackArchive(archive_path);

    ASSERT_TRUE(fs::exists(archive_path));
    ASSERT_TRUE(fs::exists(output_dir_path / input_dir_path.filename()));
    assertDirectoriesEqual(input_dir_path, output_dir_path / input_dir_path.filename());
}

TEST_F(DirectoryCompressorTests, canCompressAndDecompressBigDirectory) {
    setupInputDir();
    createLargeFile();

    spdlog::info("Huge file size: {}", std::filesystem::file_size(input_dir_path / "huge_file.bin"));
    DirectoryCompressor dc1{input_dir_path};
    dc1.createArchive(archive_path);
    spdlog::info("Archive size: {}", std::filesystem::file_size(archive_path));

    DirectoryCompressor dc2{output_dir_path};
    std::filesystem::create_directories(output_dir_path);
    dc2.unpackArchive(archive_path);

    ASSERT_TRUE(fs::exists(archive_path));
    ASSERT_TRUE(fs::exists(output_dir_path / input_dir_path.filename()));
    assertDirectoriesEqual(input_dir_path, output_dir_path / input_dir_path.filename());
}
