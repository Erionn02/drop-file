#include <gtest/gtest.h>

#include "TestHelpers.hpp"
#include "gzip.hpp"

#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>


using namespace ::testing;


struct GzipTests: public Test {
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



TEST_F(GzipTests, canCompressAndDecompress) {
    std::stringstream output_stream;
    std::ifstream input_file{input_data_path, std::ios::binary};
    gzip::compress(input_file, output_stream);

    auto compressed_data = output_stream.str();

    std::stringstream compressed_data_stream{};
    std::stringstream decompressed_data_stream{};

    compressed_data_stream << compressed_data;

    gzip::decompress(decompressed_data_stream, compressed_data_stream, compressed_data.size());
    ASSERT_EQ(decompressed_data_stream.str(), input_data);
}

TEST_F(GzipTests, canCompressAndDecompressLargeAmountsOfData) {
    std::stringstream output_stream;
    std::stringstream input_stream;
    input_data = generateRandomString(300'000'000); // 300 MB
    input_stream << input_data;
    gzip::compress(input_stream, output_stream);

    auto compressed_data = output_stream.str();

    std::stringstream compressed_data_stream{};
    std::stringstream decompressed_data_stream{};

    compressed_data_stream << compressed_data;
    gzip::decompress(decompressed_data_stream, compressed_data_stream, compressed_data.size());

    ASSERT_EQ(decompressed_data_stream.str(), input_data);
}

TEST_F(GzipTests, canCompressAndDecompressWhenEmptyInputData) {
    input_data = "";
    std::fstream input_file{input_data_path, std::ios::binary | std::ios::trunc};

    std::stringstream output_stream;
    gzip::compress(input_file, output_stream);

    auto compressed_data = output_stream.str();

    std::stringstream compressed_data_stream{};
    std::stringstream decompressed_data_stream{};

    compressed_data_stream << compressed_data;

    gzip::decompress(decompressed_data_stream, compressed_data_stream, compressed_data.size());
    std::string decompressed_data = decompressed_data_stream.str();
    ASSERT_EQ(decompressed_data.size(), 0);
    ASSERT_EQ(input_data.size(), 0);
}