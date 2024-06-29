#include <gtest/gtest.h>

#include "DirectoryCompressor.hpp"

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

    }
}; 

TEST_F(DirectoryCompressorTests, canCompressAndDecompress) {
    setupInputDir();

    DirectoryCompressor dc1{input_dir_path};
    dc1.compress(archive_path);

    DirectoryCompressor dc2{output_dir_path};
    dc1.decompress(archive_path);
    
    ASSERT_TRUE(fs::exists(archive_path));
    ASSERT_TRUE(fs::exists(output_dir_path));
}
