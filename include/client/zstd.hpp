#pragma once

#include "DropFileBaseException.hpp"


#include <string>
#include <fstream>
#include <stdexcept>
#include <array>
#include <functional>


class ZSTDException : public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};

class zstd {
public:
    static size_t compress(std::istream &input_stream, std::ostream &output_stream,
                           std::function<void()> update_callback = [] {});

    static std::size_t decompress(std::ostream &decompressed_out_stream, std::istream &compressed_in_stream,
                                  std::size_t compressed_length);
};


