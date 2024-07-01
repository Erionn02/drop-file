#include "DirectoryCompressor.hpp"

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/copy.hpp>
#include <fmt/format.h>

namespace io = boost::iostreams;



DirectoryCompressor::DirectoryCompressor(fs::path directory): directory(std::move(directory)) {}


void DirectoryCompressor::decompress(const fs::path &zip_file_path) {
    if (fs::exists(directory)) {
        throw DirectoryCompressorException(fmt::format("Directory that you try to decompress into ({}) already exists!", zip_file_path.string()));
    }
    std::ifstream ifs(zip_file_path, std::ios_base::binary);
    io::filtering_streambuf<io::input> in;
    in.push(io::zlib_decompressor());
    in.push(ifs);

    std::stringstream decompressed_stream;
    io::copy(in, decompressed_stream);

    std::istringstream archive_input_stream(decompressed_stream.str());
    boost::archive::text_iarchive archive(archive_input_stream);
    deserializeArchive(archive);
}

void DirectoryCompressor::deserializeArchive(boost::archive::text_iarchive &archive) {
    while (true) {
        try {
            DirEntryInfo info;
            archive >> info;

            fs::path path = directory / info.path;
            if (info.is_directory) {
                fs::create_directories(path);
            } else {
                fs::create_directories(path.parent_path());

                std::ofstream ofs(path.string(), std::ios_base::binary);
                ofs << info.content;
            }
        } catch (const boost::archive::archive_exception& e) {
            if (e.code == boost::archive::archive_exception::input_stream_error) {
                break;  // End of archive
            } else {
                throw;
            }
        }
    }
}

void DirectoryCompressor::compress(const fs::path &new_zip_file_path) {
    if (fs::exists(new_zip_file_path)) {
        throw DirectoryCompressorException(fmt::format("File that you try to decompress to ({}) already exists!", new_zip_file_path.string()));
    }
    std::ofstream ofs(new_zip_file_path, std::ios_base::binary);
    io::filtering_streambuf<io::output> out;
    out.push(io::zlib_compressor());
    out.push(ofs);

    std::stringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    compressDirectory(directory, archive);

    std::istringstream archive_input_stream(archive_stream.str());
    io::copy(archive_input_stream, out);
}

void DirectoryCompressor::compressDirectory(const fs::path &dir_to_compress, boost::archive::text_oarchive &archive,
                                            const fs::path &relative_path) {
    addDirectoryToArchive(archive, relative_path);
    for (const auto& dir_entry: fs::directory_iterator(dir_to_compress)) {
        const fs::path& current = dir_entry.path();
        fs::path relative = relative_path / current.filename();

        if (fs::is_directory(current)) {
            compressDirectory(current, archive, relative);
        } else if (fs::is_regular_file(current)) {
            compressFile(archive, current, relative);
        }
    }
}

void DirectoryCompressor::addDirectoryToArchive(boost::archive::text_oarchive &archive,
                                                const fs::path &relative_path) const {
    DirEntryInfo info;
    info.path = relative_path;
    info.is_directory = true;
    archive << info;
}

void
DirectoryCompressor::compressFile(boost::archive::text_oarchive &archive, const fs::path &current, const fs::path &relative) const {
    DirEntryInfo info;
    info.path = relative.string();
    info.is_directory = false;
    std::ifstream ifs(current.string(), std::ios_base::binary);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    info.content = buffer.str();

    archive << info;
}
