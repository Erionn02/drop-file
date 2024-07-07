#include "DirectoryCompressor.hpp"
#include "Utils.hpp"

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/copy.hpp>
#include <fmt/format.h>

namespace io = boost::iostreams;



DirectoryCompressor::DirectoryCompressor(fs::path directory) : directory(std::move(directory)),
                                                               progress_bar(indicators::option::BarWidth{40},
                                                                            indicators::option::Start{"["},
                                                                            indicators::option::Fill{"·"},
                                                                            indicators::option::Lead{"<==>"},
                                                                            indicators::option::End{"]"},
                                                                            indicators::option::ForegroundColor{
                                                                                    indicators::Color::yellow},
                                                                            indicators::option::FontStyles{
                                                                                    std::vector<indicators::FontStyle>{
                                                                                            indicators::FontStyle::bold}}) {}


void DirectoryCompressor::decompress(const fs::path &zip_file_path) {
    if (fs::exists(directory)) {
        throw DirectoryCompressorException(
                fmt::format("Directory that you try to decompress into ({}) already exists!", zip_file_path.string()));
    }
    std::ifstream ifs(zip_file_path, std::ios_base::binary);
    io::filtering_streambuf<io::input> in;
    in.push(io::zlib_decompressor());
    in.push(ifs);

    std::stringstream decompressed_stream;
    io::copy(in, decompressed_stream);

    std::istringstream archive_input_stream(decompressed_stream.str());
    boost::archive::text_iarchive archive(archive_input_stream);
    checkLeftDiskSpace(zip_file_path, archive);

    progress_bar.set_option(indicators::option::PrefixText {"Decompressing..."});
    deserializeArchive(archive);
    progress_bar.set_option(indicators::option::PrefixText {"Decompressed."});
    progress_bar.mark_as_completed();
}

void
DirectoryCompressor::checkLeftDiskSpace(const fs::path &zip_file_path,
                                        boost::archive::text_iarchive &archive) const {
    std::size_t original_size;
    archive >> original_size;
    auto left_disk_space = std::filesystem::space(directory.parent_path()).free;
    if (left_disk_space < original_size) {
        throw DirectoryCompressorException(fmt::format("Not enough space to decompress {}.", zip_file_path.string()));
    }
}

void DirectoryCompressor::deserializeArchive(boost::archive::text_iarchive &archive) {
    while (true) {
        try {
            DirEntryInfo info;
            archive >> info;

            fs::path path = directory / info.path;
            progress_bar.tick();
            if (info.is_directory) {
                fs::create_directories(path);
            } else {
                fs::create_directories(path.parent_path());

                std::ofstream ofs(path.string(), std::ios_base::binary);
                ofs << info.content;
            }
        } catch (const boost::archive::archive_exception &e) {
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
        throw DirectoryCompressorException(
                fmt::format("File that you try to decompress to ({}) already exists!", new_zip_file_path.string()));
    }
    std::ofstream ofs(new_zip_file_path, std::ios_base::binary);
    io::filtering_streambuf<io::output> out;
    out.push(io::zlib_compressor());
    out.push(ofs);

    std::stringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    progress_bar.set_option(indicators::option::PrefixText{"Compressing directory..."});

    addOriginalDirSizeToArchive(archive);
    compressDirectory(directory, archive);

    std::istringstream archive_input_stream(archive_stream.str());
    io::copy(archive_input_stream, out);

    progress_bar.set_option(indicators::option::PrefixText{"Directory compressed."});
    progress_bar.mark_as_completed();
}

void DirectoryCompressor::addOriginalDirSizeToArchive(boost::archive::text_oarchive &archive) const {
    std::size_t directory_size = getDirectorySize(directory);
    archive & directory_size;
}

void DirectoryCompressor::compressDirectory(const fs::path &dir_to_compress, boost::archive::text_oarchive &archive,
                                            const fs::path &relative_path) {
    addDirectoryToArchive(archive, relative_path);
    for (const auto &dir_entry: fs::directory_iterator(dir_to_compress)) {
        progress_bar.tick();
        const fs::path &current = dir_entry.path();
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
    info.path = relative_path.string();
    info.is_directory = true;
    archive << info;
}

void
DirectoryCompressor::compressFile(boost::archive::text_oarchive &archive, const fs::path &current,
                                  const fs::path &relative) const {
    DirEntryInfo info;
    info.path = relative.string();
    info.is_directory = false;
    std::ifstream ifs(current.string(), std::ios_base::binary);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    info.content = buffer.str();

    archive << info;
}
