#pragma once
#include "DropFileBaseException.hpp"
#include "DirEntryInfo.hpp"

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


class DirectoryCompressor {
public:
    DirectoryCompressor(fs::path directory);

    void createArchive(const fs::path& new_archive_path);
    void unpackArchive(const fs::path& archive_path);

private:
    void decompressFile(std::ifstream &compressed_file, const DirEntryInfo &entry_info);
    void compressDirectory(const fs::path &dir_to_compress, std::ofstream& new_archive,
                           const fs::path &relative_path = "");
    void compressFile(const fs::path &file_path, std::ofstream &compressed_archive, const fs::path &relative_path);
    void addDirectory(std::ofstream &new_archive, const fs::path &relative_path);

    static constexpr std::size_t BUFFER_SIZE{1024*1024};


    fs::path directory;
    indicators::IndeterminateProgressBar progress_bar;
};


