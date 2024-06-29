#include <gtest/gtest.h>

#include "DirectoryCompressor.hpp"

#include <boost/range/combine.hpp>


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
        fs::create_directories(input_dir_path / "another_nested_dir"); // todo add support for empty dirs
    }

    void assertDirectoriesEqual(const fs::path& dir1, const fs::path& dir2) {
        auto dir_entries_1 = getSortedDirEntries(dir1);
        auto dir_entries_2 = getSortedDirEntries(dir2);
        ASSERT_EQ(dir_entries_1.size(), dir_entries_2.size());
        for(auto [el1, el2]: boost::range::combine(dir_entries_1, dir_entries_2)) { // std::views::zip requires gcc 13 :(
            ASSERT_EQ(el1.path().filename(), el1.path().filename());
            ASSERT_EQ(el1.is_directory(), el2.is_directory());
            ASSERT_EQ(el1.is_regular_file(), el2.is_regular_file());
            if (el1.is_directory()) {
                assertDirectoriesEqual(el1, el2);
            } else {
                ASSERT_EQ(getFileContent(el1), getFileContent(el2));
            }
        }
    }

    // normal iterating over directory does not guarantee any particular order of entries,
    // therefore I have to sort it first
    std::set<fs::directory_entry> getSortedDirEntries(const fs::path& dir) {
        return {fs::directory_iterator(dir), fs::directory_iterator()};
    }

    std::string getFileContent(const fs::path& file_path) {
        std::ifstream file{file_path, std::ios::binary};
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }
}; 

TEST_F(DirectoryCompressorTests, canCompressAndDecompress) {
    setupInputDir();

    DirectoryCompressor dc1{input_dir_path};
    dc1.compress(archive_path);

    DirectoryCompressor dc2{output_dir_path};
    dc2.decompress(archive_path);
    
    ASSERT_TRUE(fs::exists(archive_path));
    ASSERT_TRUE(fs::exists(output_dir_path));
    assertDirectoriesEqual(input_dir_path, output_dir_path);
}

TEST_F(DirectoryCompressorTests, canCompressAndDecompressEmptyDir) {
    std::filesystem::create_directories(input_dir_path);

    DirectoryCompressor dc1{input_dir_path};
    dc1.compress(archive_path);

    DirectoryCompressor dc2{output_dir_path};
    dc2.decompress(archive_path);

    ASSERT_TRUE(fs::exists(archive_path));
    ASSERT_TRUE(fs::exists(output_dir_path));
    assertDirectoriesEqual(input_dir_path, output_dir_path);
}
