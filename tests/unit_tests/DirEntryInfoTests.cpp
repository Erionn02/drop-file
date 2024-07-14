#include <gtest/gtest.h>

#include "DirEntryInfo.hpp"

#include <fstream>


using namespace ::testing;



struct DirEntryInfoTests: public Test{
    std::filesystem::path input_archive_path{std::filesystem::temp_directory_path() / "input_archive"};
    std::filesystem::path output_archive_path{std::filesystem::temp_directory_path() / "output_archive"};

    void SetUp() override {

    }

    void TearDown() override {
        std::filesystem::remove_all(input_archive_path);
        std::filesystem::remove_all(output_archive_path);
    }
};

TEST_F(DirEntryInfoTests, canWriteAndReadFromStream) {
    std::fstream archive{output_archive_path, std::ios::binary | std::ios::trunc};

    bool is_directory = true;
    std::filesystem::path relative_path{"./path"};
    DirEntryInfo entry_info{is_directory, relative_path};

    entry_info.writeToStream(archive);
}