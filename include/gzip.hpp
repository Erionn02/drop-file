#pragma once

#include "DropFileBaseException.hpp"

#include <zlib.h>

#include <string>
#include <fstream>
#include <stdexcept>
#include <array>
#include <functional>


class GzipException : public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};

class gzip {
public:
    static size_t compress(std::istream &input_stream, std::ostream &output_stream,
                           std::function<void()> update_callback = [] {});

    static std::size_t decompress(std::ostream &decompressed_out_stream, std::istream &compressed_in_stream,
                                  std::size_t compressed_length);

private:
    static constexpr std::size_t BUFFER_SIZE{3 * 1024 * 1024};

    static std::size_t
    compressChunk(std::ostream &output_stream, const std::istream &input_stream, z_stream &deflate_stream,
                  std::array<char, BUFFER_SIZE> &input_buffer,
                  std::array<unsigned char, BUFFER_SIZE> &compression_buffer, int flush);
};


