#include <gtest/gtest.h>

#include "TestHelpers.hpp"
#include "gzip.hpp"

#include <filesystem>
#include <fstream>


using namespace ::testing;



struct GzipTests: public Test {
    const std::filesystem::path compressed_data_path{std::filesystem::temp_directory_path() / "compressed_data.gz"};
    const std::filesystem::path decompressed_data_path{std::filesystem::temp_directory_path() / "decompressed_data"};
    const std::filesystem::path input_data_path{std::filesystem::temp_directory_path() / "input_data"};
    std::string input_data{generateRandomString(1000000)};

    void SetUp() override {
        std::ofstream input_file{input_data_path, std::ios::binary | std::ios::trunc};
        input_file << input_data;
    }

    void TearDown() override {
        std::filesystem::remove(compressed_data_path);
        std::filesystem::remove(decompressed_data_path);
        std::filesystem::remove(input_data_path);
    }

    std::string decompress(std::size_t size) {
        std::ifstream f{compressed_data_path, std::ios::binary};
        std::stringstream decompressed_data_stream{};
        gzip::decompress(decompressed_data_stream, f, size);

        return decompressed_data_stream.str();
    }

};



TEST_F(GzipTests, canCompressAndDecompress) {
    std::stringstream output_stream;
    gzip::compress(input_data_path, output_stream);

    auto compressed_data = output_stream.str();

    std::stringstream compressed_data_stream{};
    std::stringstream decompressed_data_stream{};

    compressed_data_stream << compressed_data;

    gzip::decompress(decompressed_data_stream, compressed_data_stream, compressed_data.size());
    ASSERT_EQ(decompressed_data_stream.str(), input_data);
}

TEST_F(GzipTests, canCompressAndDecompressWhenEmptyInputData) {
    input_data = "";
    std::ofstream input_file{input_data_path, std::ios::binary | std::ios::trunc};

    std::stringstream output_stream;
    gzip::compress(input_data_path, output_stream);

    auto compressed_data = output_stream.str();

    std::stringstream compressed_data_stream{};
    std::stringstream decompressed_data_stream{};

    compressed_data_stream << compressed_data;

    gzip::decompress(decompressed_data_stream, compressed_data_stream, compressed_data.size());
    std::string decompressed_data = decompressed_data_stream.str();
    ASSERT_EQ(decompressed_data.size(), 0);
    ASSERT_EQ(input_data.size(), 0);
}