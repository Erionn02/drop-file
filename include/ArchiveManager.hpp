#pragma once
#include "DropFileBaseException.hpp"
#include "FSEntryInfo.hpp"

#include <indicators/indeterminate_progress_bar.hpp>
#include <zlib.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class ArchiveManagerException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};


class ArchiveManager {
public:
    ArchiveManager(fs::path directory);

    void createArchive(const fs::path& new_archive_path);
    void unpackArchive(const fs::path& archive_path);

private:
    void unpackFile(std::ifstream &compressed_archive, const FSEntryInfo &entry_info);
    void packDirectory(const fs::path &dir_to_compress, std::ofstream& new_archive,
                       const fs::path &relative_path = "");
    void addFile(const fs::path &file_path, std::ofstream &compressed_archive, const fs::path &relative_path);
    void addDirectory(std::ofstream &new_archive, const fs::path &relative_path);


    fs::path directory;
    indicators::IndeterminateProgressBar progress_bar;
};


