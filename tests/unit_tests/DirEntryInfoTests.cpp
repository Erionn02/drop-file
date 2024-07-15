#include <gtest/gtest.h>

#include "DirEntryInfo.hpp"
#include "TestHelpers.hpp"

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

TEST_F(DirEntryInfoTests, throwsOnTooLongPath) {
    bool is_directory = true;
    std::filesystem::path relative_path{generateRandomString(DirEntryInfo::PATH_MAX_LEN + 1)};

    ASSERT_THROW((DirEntryInfo{is_directory, relative_path}), DirEntryInfoException);
}


TEST_F(DirEntryInfoTests, canWriteAndReadFromStream) {
    std::stringstream archive;

    bool is_directory = true;
    std::filesystem::path relative_path{"./path"};
    DirEntryInfo entry_info{is_directory, relative_path};

    entry_info.writeToStream(archive);
    auto read_from_stream = DirEntryInfo::readFromStream(archive, 12345);
    ASSERT_EQ(read_from_stream, entry_info);
    ASSERT_EQ(read_from_stream.relative_path, relative_path.string());
    ASSERT_EQ(read_from_stream.compressed_length, 0);
    ASSERT_EQ(read_from_stream.is_directory, is_directory);
}

TEST_F(DirEntryInfoTests, returnsCorrectPosition) {
    std::stringstream archive;

    bool is_directory = false;
    std::filesystem::path relative_path{"./path"};
    DirEntryInfo entry_info{is_directory, relative_path};

    auto size_pos = entry_info.writeToStream(archive);
    std::size_t new_size{2136};
    entry_info.writeCompressedLength(archive, size_pos, new_size);
    auto read_from_stream = DirEntryInfo::readFromStream(archive, 12345);
    ASSERT_EQ(read_from_stream.compressed_length, new_size);
}

TEST_F(DirEntryInfoTests, throwsWhenStreamIsEmpty) {
    std::stringstream archive;

    ASSERT_THROW((DirEntryInfo::readFromStream(archive, 12345)), DirEntryInfoException);
}

TEST_F(DirEntryInfoTests, throwsWhenThereIsNotEnoughBytesForPathLength) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));

    ASSERT_THROW((DirEntryInfo::readFromStream(archive, 12345)), DirEntryInfoException);
}

TEST_F(DirEntryInfoTests, throwsWhenPathIsTooLong) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::size_t path_length{DirEntryInfo::PATH_MAX_LEN + 1};
    archive.write(std::bit_cast<char *>(&path_length), sizeof(path_length));

    ASSERT_THROW((DirEntryInfo::readFromStream(archive, 12345)), DirEntryInfoException);
}

TEST_F(DirEntryInfoTests, throwsWhenThereIsNotEnoughBytesForCompressedSize) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::string path{"some path"};
    auto path_length = path.size();
    archive.write(std::bit_cast<char *>(&path_length), sizeof(path_length));
    archive << path;

    ASSERT_THROW((DirEntryInfo::readFromStream(archive, 12345)), DirEntryInfoException);
}

TEST_F(DirEntryInfoTests, throwsWhenCompressedLengthIsBiggerThanBytesLeftInStream) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::string path{"some path"};
    auto path_length = path.size();
    archive.write(std::bit_cast<char *>(&path_length), sizeof(path_length));
    archive << path;
    std::size_t compressed_length{1234};
    archive.write(std::bit_cast<char *>(&compressed_length), sizeof(compressed_length));

    ASSERT_THROW((DirEntryInfo::readFromStream(archive, compressed_length)), DirEntryInfoException);
}

TEST_F(DirEntryInfoTests, noThrowWhenAllGood) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::string path{"some path"};
    auto path_length = path.size();
    archive.write(std::bit_cast<char *>(&path_length), sizeof(path_length));
    archive << path;
    std::size_t compressed_length{1234};
    archive.write(std::bit_cast<char *>(&compressed_length), sizeof(compressed_length));


    auto read_from_stream = DirEntryInfo::readFromStream(archive, 12345);
    ASSERT_EQ(read_from_stream.relative_path, path);
    ASSERT_EQ(read_from_stream.compressed_length, compressed_length);
    ASSERT_EQ(read_from_stream.is_directory, is_directory);
}