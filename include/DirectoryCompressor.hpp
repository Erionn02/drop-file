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
    DirectoryCompressor(std::filesystem::path directory);

    void decompress(const std::filesystem::path& zip_file_path);
    void compress(const std::filesystem::path& new_zip_file_path);

private:
    void compressDirectory(const fs::path &dir_to_compress, boost::archive::text_oarchive &archive,
                           const fs::path &relative_path = "");
    void compressFile(boost::archive::text_oarchive &archive, const fs::path &current, const fs::path &relative) const;

    std::filesystem::path directory;

    struct FileInfo {
        std::string path;
        std::string content;

        template<class Archive>
        void serialize(Archive &archive, const unsigned int) {
            archive & path;
            archive & content;
        }
    };
};


