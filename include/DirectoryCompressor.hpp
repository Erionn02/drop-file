#pragma once
#include "DropFileBaseException.hpp"

#include <indicators/indeterminate_progress_bar.hpp>
#include <zlib.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class DirectoryCompressorException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};

#pragma pack(push, 1)
struct DirEntryInfo {
    DirEntryInfo() = default;
    DirEntryInfo(bool is_directory, const fs::path &relative_path);

    bool is_directory{};
    std::string relative_path{};
    std::size_t compressed_length{0};

    static DirEntryInfo readFromStream(std::ifstream &stream, std::size_t total_file_length);
    std::streamsize writeToStream(std::ofstream &stream);
    void writeCompressedLength(std::ofstream &stream, std::streamsize pos, std::size_t total_written);

    static constexpr std::size_t PATH_MAX_LEN{PATH_MAX};
};
#pragma pack(pop)

class DirectoryCompressor {
public:
    DirectoryCompressor(fs::path directory);

    void decompress(const fs::path& compressed_file_path);
    void compress(const fs::path& new_compressed_file_path);

private:
    void decompressFile(std::ifstream &compressed_file, const DirEntryInfo &entry_info);
    void compressDirectory(const fs::path &dir_to_compress, std::ofstream& compressed_archive,
                           const fs::path &relative_path = "");
    void compressFile(const fs::path &file_path, std::ofstream &compressed_archive, const fs::path &relative_path);
    void addDirectory(std::ofstream &out_file, const fs::path &relative_path);

    static constexpr std::size_t BUFFER_SIZE{1024*1024};
    std::pair<int, std::size_t> compressChunk(std::ofstream &compressed_file, z_stream_s &zs,
                                              std::array<unsigned char, BUFFER_SIZE> &compression_buffer, int flush);


    fs::path directory;
    indicators::IndeterminateProgressBar progress_bar;
};


