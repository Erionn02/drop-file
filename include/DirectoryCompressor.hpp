#pragma once
#include "DropFileBaseException.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <indicators/indeterminate_progress_bar.hpp>


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

    void decompress(const fs::path& zip_file_path);
    void compress(const fs::path& new_zip_file_path);

private:
    void compressDirectory(const fs::path &dir_to_compress, boost::archive::text_oarchive &archive,
                           const fs::path &relative_path = "");
    void addDirectoryToArchive(boost::archive::text_oarchive &archive, const fs::path &relative_path) const;
    void compressFile(boost::archive::text_oarchive &archive, const fs::path &current, const fs::path &relative) const;
    void deserializeArchive(boost::archive::text_iarchive &archive);
    void addOriginalDirSizeToArchive(boost::archive::text_oarchive &archive) const;
    void checkLeftDiskSpace(const fs::path &zip_file_path, boost::archive::text_iarchive &archive) const;

    fs::path directory;
    indicators::IndeterminateProgressBar progress_bar;

    struct DirEntryInfo {
        std::string path;
        std::string content;
        bool is_directory{false};

        template<class Archive>
        void serialize(Archive &archive, const unsigned int) {
            archive & path;
            archive & content;
            archive & is_directory;
        }
    };
};


