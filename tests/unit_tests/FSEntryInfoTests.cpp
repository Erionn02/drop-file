#include <gtest/gtest.h>

#include "client/FSEntryInfo.hpp"
#include "TestHelpers.hpp"

#include <fstream>


using namespace ::testing;



struct FSEntryInfoTests: public Test{
    std::filesystem::path input_archive_path{std::filesystem::temp_directory_path() / "input_archive"};
    std::filesystem::path output_archive_path{std::filesystem::temp_directory_path() / "output_archive"};


    void TearDown() override {
        std::filesystem::remove_all(input_archive_path);
        std::filesystem::remove_all(output_archive_path);
    }
};

TEST_F(FSEntryInfoTests, throwsOnTooLongPath) {
    bool is_directory = true;
    std::filesystem::path relative_path{generateRandomString(FSEntryInfo::PATH_MAX_LEN + 1)};

    ASSERT_THROW((FSEntryInfo{is_directory, relative_path}), FSEntryInfoException);
}


TEST_F(FSEntryInfoTests, canWriteAndReadFromStream) {
    std::stringstream archive;

    bool is_directory = true;
    std::filesystem::path relative_path{"./path"};
    FSEntryInfo entry_info{is_directory, relative_path};

    entry_info.writeToStream(archive);
    auto read_from_stream = FSEntryInfo::readFromStream(archive, 8088);
    ASSERT_EQ(read_from_stream, entry_info);
    ASSERT_EQ(read_from_stream.relative_path, relative_path.string());
    ASSERT_EQ(read_from_stream.compressed_length, 0);
    ASSERT_EQ(read_from_stream.is_directory, is_directory);
}

TEST_F(FSEntryInfoTests, returnsCorrectPosition) {
    std::stringstream archive;

    bool is_directory = false;
    std::filesystem::path relative_path{"./path"};
    FSEntryInfo entry_info{is_directory, relative_path};

    auto size_pos = entry_info.writeToStream(archive);
    std::size_t new_size{2136};
    entry_info.writeCompressedLength(archive, size_pos, new_size);
    auto read_from_stream = FSEntryInfo::readFromStream(archive, 8088);
    ASSERT_EQ(read_from_stream.compressed_length, new_size);
}

TEST_F(FSEntryInfoTests, throwsWhenStreamIsEmpty) {
    std::stringstream archive;

    ASSERT_THROW((FSEntryInfo::readFromStream(archive, 8088)), FSEntryInfoException);
}

TEST_F(FSEntryInfoTests, throwsWhenThereIsNotEnoughBytesForPathLength) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));

    ASSERT_THROW((FSEntryInfo::readFromStream(archive, 8088)), FSEntryInfoException);
}

TEST_F(FSEntryInfoTests, throwsWhenPathIsTooLong) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::size_t path_length{FSEntryInfo::PATH_MAX_LEN + 1};
    archive.write(std::bit_cast<char *>(&path_length), sizeof(path_length));

    ASSERT_THROW((FSEntryInfo::readFromStream(archive, 8088)), FSEntryInfoException);
}

TEST_F(FSEntryInfoTests, throwsWhenThereIsNotEnoughBytesForCompressedSize) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::string path{"some path"};
    auto path_length = path.size();
    archive.write(std::bit_cast<char *>(&path_length), sizeof(path_length));
    archive << path;

    ASSERT_THROW((FSEntryInfo::readFromStream(archive, 8088)), FSEntryInfoException);
}

TEST_F(FSEntryInfoTests, throwsWhenCompressedLengthIsBiggerThanBytesLeftInStream) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::string path{"some path"};
    auto path_length = path.size();
    archive.write(std::bit_cast<char *>(&path_length), sizeof(path_length));
    archive << path;
    std::size_t compressed_length{1234};
    archive.write(std::bit_cast<char *>(&compressed_length), sizeof(compressed_length));

    ASSERT_THROW((FSEntryInfo::readFromStream(archive, compressed_length)), FSEntryInfoException);
}

TEST_F(FSEntryInfoTests, noThrowWhenAllGood) {
    std::stringstream archive;
    bool is_directory = false;
    archive.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::string path{"some path"};
    auto path_length = path.size();
    archive.write(std::bit_cast<char *>(&path_length), sizeof(path_length));
    archive << path;
    std::size_t compressed_length{1234};
    archive.write(std::bit_cast<char *>(&compressed_length), sizeof(compressed_length));

    auto read_from_stream = FSEntryInfo::readFromStream(archive, 8088);
    ASSERT_EQ(read_from_stream.relative_path, path);
    ASSERT_EQ(read_from_stream.compressed_length, compressed_length);
    ASSERT_EQ(read_from_stream.is_directory, is_directory);
}