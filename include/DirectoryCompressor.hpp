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

    bool is_directory;
    char relative_path[PATH_MAX];
    std::size_t compressed_length{0};

    static inline std::size_t PATH_MAX_LEN{PATH_MAX};
    static inline std::size_t MAX_FILE_LENGTH{1024ull * 1024ull * 1024ull * 4ull}; // 4 GB
};
#pragma pack(pop)

class DirectoryCompressor {
public:
    DirectoryCompressor(fs::path directory);

    void decompress(const fs::path& compressed_file_path);
    void compress(const fs::path& new_compressed_file_path);

private:
    void decompressFile(std::ifstream &compressed_file, const DirEntryInfo &entry_info);

    DirEntryInfo readDirEntryInfo(std::ifstream &zip_file, std::size_t total_file_length);

    void compressDirectory(const fs::path &dir_to_compress, std::ofstream& compressed_archive,
                           const fs::path &relative_path = "");
    void compressFile(const fs::path &file_path, std::ofstream &compressed_archive, const fs::path &relative_path);
    void addDirectory(std::ofstream &out_file, const fs::path &relative_path);
    size_t getLeftBytes(std::ifstream &zip_file, size_t total_file_length) const;

    static constexpr std::size_t BUFFER_SIZE{1024*1024};
    std::pair<int, std::size_t> compressChunk(std::ofstream &compressed_file, z_stream_s &zs,
                                              std::array<unsigned char, BUFFER_SIZE> &compression_buffer, int flush);


    fs::path directory;
    indicators::IndeterminateProgressBar progress_bar;
};


