#pragma once

#include <iostream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;


#include <filesystem>


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

    fs::path directory;

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


