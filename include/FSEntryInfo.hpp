#pragma once
#include "DropFileBaseException.hpp"

#include <iostream>
#include <vector>
#include <filesystem>
#include <stdexcept>


class FSEntryInfoException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};


#pragma pack(push, 1)
struct FSEntryInfo {
    FSEntryInfo() = default;
    FSEntryInfo(bool is_directory, const std::filesystem::path &relative_path);
    bool operator==(const FSEntryInfo&) const = default;

    static FSEntryInfo readFromStream(std::istream &stream, std::size_t total_stream_size);
    std::streamsize writeToStream(std::ostream &stream);
    void writeCompressedLength(std::ostream &stream, std::streamsize pos, std::size_t total_written);


    bool is_directory{};
    std::string relative_path{};
    std::size_t compressed_length{0};
    static constexpr std::size_t PATH_MAX_LEN{4096};
};
#pragma pack(pop)

