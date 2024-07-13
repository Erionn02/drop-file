#pragma once
#include <string>
#include <fstream>
#include <stdexcept>

class GzipException: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class gzip {
public:
    static std::size_t compress(const std::string& input_path, std::ostream &output_stream);

    static std::size_t decompress(std::ostream &decompressed_out_stream,
                                  std::istream &compressed_in_stream,
                                  std::size_t compressed_length);

    static constexpr std::size_t BUFFER_SIZE {1024*1024};
};


