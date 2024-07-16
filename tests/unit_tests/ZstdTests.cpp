#include <gtest/gtest.h>

#include "TestHelpers.hpp"
#include "zstd.hpp"

#include <filesystem>
#include <fstream>


using namespace ::testing;


struct ZstdTests: public Test {
    const std::filesystem::path input_data_path{std::filesystem::temp_directory_path() / "input_data"};
    std::string input_data{generateRandomString(1'000'000)};

    void SetUp() override {
        std::ofstream input_file{input_data_path, std::ios::binary | std::ios::trunc};
        input_file << input_data;
    }

    void TearDown() override {
        std::filesystem::remove(input_data_path);
    }
};



TEST_F(ZstdTests, canCompressAndDecompress) {
    std::stringstream output_stream;
    std::ifstream input_file{input_data_path, std::ios::binary};
    zstd::compress(input_file, output_stream);

    auto compressed_data = output_stream.str();

    std::stringstream compressed_data_stream{};
    std::stringstream decompressed_data_stream{};

    compressed_data_stream << compressed_data;

    zstd::decompress(decompressed_data_stream, compressed_data_stream, compressed_data.size());
    ASSERT_EQ(decompressed_data_stream.str(), input_data);
}

TEST_F(ZstdTests, canCompressAndDecompressLargeAmountsOfData) {
    std::stringstream output_stream;
    std::stringstream input_stream;
    input_data = generateRandomString(300'000'000); // 300 MB
    input_stream << input_data;
    zstd::compress(input_stream, output_stream);

    auto compressed_data = output_stream.str();

    std::stringstream compressed_data_stream{};
    std::stringstream decompressed_data_stream{};

    compressed_data_stream << compressed_data;
    zstd::decompress(decompressed_data_stream, compressed_data_stream, compressed_data.size());

    ASSERT_EQ(decompressed_data_stream.str(), input_data);
}

TEST_F(ZstdTests, canCompressAndDecompressWhenEmptyInputData) {
    input_data = "";
    std::fstream input_file{input_data_path, std::ios::binary | std::ios::trunc};

    std::stringstream output_stream;
    zstd::compress(input_file, output_stream);

    auto compressed_data = output_stream.str();

    std::stringstream compressed_data_stream{};
    std::stringstream decompressed_data_stream{};

    compressed_data_stream << compressed_data;

    zstd::decompress(decompressed_data_stream, compressed_data_stream, compressed_data.size());
    std::string decompressed_data = decompressed_data_stream.str();
    ASSERT_EQ(decompressed_data.size(), 0);
    ASSERT_EQ(input_data.size(), 0);
}