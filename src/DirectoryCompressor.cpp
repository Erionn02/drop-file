#include "DirectoryCompressor.hpp"
#include "Utils.hpp"
#include "gzip.hpp"

#include <fmt/format.h>
#include <spdlog/spdlog.h>


size_t getLeftBytes(std::ifstream &zip_file, size_t total_file_length);


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


void DirectoryCompressor::decompress(const fs::path &compressed_file_path) {
    if (!fs::exists(directory)) {
        throw DirectoryCompressorException(
                fmt::format("Directory that you try to decompress into ({}) does not exists!",
                            compressed_file_path.string()));
    }
    if (!fs::exists(compressed_file_path)) {
        throw DirectoryCompressorException(
                fmt::format("Given archive {} does not exists!", compressed_file_path.string()));
    }
    std::ifstream compressed_archive(compressed_file_path, std::ios_base::binary);
    std::size_t file_size = std::filesystem::file_size(compressed_file_path);

    progress_bar.set_option(indicators::option::PrefixText{"Decompressing..."});
    while (getLeftBytes(compressed_archive, file_size) > 0) {
        DirEntryInfo entry_info = DirEntryInfo::readFromStream(compressed_archive, file_size);
        spdlog::info("Decompressing, path: {}, compressed size: {}, is_dir: {}", entry_info.relative_path,
                     entry_info.compressed_length, entry_info.is_directory);
        if (entry_info.is_directory) {
            std::filesystem::create_directories(directory / entry_info.relative_path);
        } else {
            decompressFile(compressed_archive, entry_info);
        }
    }

    progress_bar.set_option(indicators::option::PrefixText{"Decompressed."});
    progress_bar.mark_as_completed();
}

void DirectoryCompressor::decompressFile(std::ifstream &compressed_archive, const DirEntryInfo &entry_info) {
    std::ofstream decompressed_file{directory / entry_info.relative_path, std::ios::binary};

    if (!decompressed_file.is_open()) {
        throw DirectoryCompressorException(
                "Failed to open output decompressed_file: " + (directory / entry_info.relative_path).string());
    }
    gzip::decompress(decompressed_file,compressed_archive, entry_info.compressed_length);
}

void DirectoryCompressor::compress(const fs::path &new_compressed_file_path) {
    if (fs::exists(new_compressed_file_path)) {
        throw DirectoryCompressorException(
                fmt::format("File that you try to decompress to ({}) already exists!",
                            new_compressed_file_path.string()));
    }
    std::ofstream compressed_file(new_compressed_file_path, std::ios::binary | std::ios::trunc);
    progress_bar.set_option(indicators::option::PrefixText{"Compressing directory..."});

    compressDirectory(directory, compressed_file, directory.filename());

    progress_bar.set_option(indicators::option::PrefixText{"Directory compressed."});
    progress_bar.mark_as_completed();
}

void DirectoryCompressor::compressDirectory(const fs::path &dir_to_compress, std::ofstream &compressed_archive,
                                            const fs::path &relative_path) {
    addDirectory(compressed_archive, relative_path);
    for (const auto &dir_entry: fs::directory_iterator(dir_to_compress)) {
        progress_bar.tick();
        const fs::path &current = dir_entry.path();
        fs::path new_relative = relative_path / current.filename();

        if (fs::is_directory(current)) {
            compressDirectory(current, compressed_archive, new_relative);
        } else if (fs::is_regular_file(current)) {
            compressFile(current, compressed_archive, new_relative);
        }
    }
}

void DirectoryCompressor::addDirectory(std::ofstream &out_file, const fs::path &relative_path) {
    DirEntryInfo info{true, relative_path};
    info.writeToStream(out_file);
}

void DirectoryCompressor::compressFile(const fs::path &file_path, std::ofstream &compressed_archive,
                                       const fs::path &relative_path) {
    DirEntryInfo file_info{false, relative_path};
    auto pos_to_write_compressed_size = file_info.writeToStream(compressed_archive);
    std::size_t bytes_written = gzip::compress(file_path, compressed_archive);

    file_info.writeCompressedLength(compressed_archive, pos_to_write_compressed_size, bytes_written);
}

DirEntryInfo::DirEntryInfo(bool is_directory, const fs::path &relative_path) : is_directory(
        is_directory) {
    auto path_str = relative_path.string();
    if (path_str.size() > DirEntryInfo::PATH_MAX_LEN) {
        throw DirectoryCompressorException(
                fmt::format("{} path is longer than allowed ({}>{})", path_str, path_str.size(),
                            DirEntryInfo::PATH_MAX_LEN));
    }
    this->relative_path = std::move(path_str);
}

DirEntryInfo DirEntryInfo::readFromStream(std::ifstream &stream, std::size_t total_file_length) {
    DirEntryInfo entry_info;
    stream.read(std::bit_cast<char *>(&entry_info.is_directory), sizeof(entry_info.is_directory));
    if (stream.gcount() != sizeof(entry_info.is_directory)) {
        throw DirectoryCompressorException("Could read type of dir entry.");
    }
    std::size_t path_length{};
    stream.read(std::bit_cast<char *>(&path_length), sizeof(path_length));
    if (stream.gcount() != sizeof(path_length)) {
        throw DirectoryCompressorException("Could read path length.");
    }
    if (path_length > PATH_MAX_LEN) {
        throw DirectoryCompressorException(
                fmt::format("Path length exceeds maximum available value ({} > {})", path_length, PATH_MAX_LEN));
    }
    entry_info.relative_path.resize(path_length);
    stream.read(entry_info.relative_path.data(), static_cast<std::streamsize>(path_length));
    if (stream.gcount() != static_cast<std::streamsize>(path_length)) {
        throw DirectoryCompressorException("Could read whole path.");
    }

    stream.read(std::bit_cast<char *>(&entry_info.compressed_length), sizeof(entry_info.compressed_length));
    if (stream.gcount() != sizeof(entry_info.compressed_length)) {
        throw DirectoryCompressorException("Could read compressed length.");
    }

    size_t left_size = getLeftBytes(stream, total_file_length);
    if (entry_info.compressed_length > left_size) {
        throw DirectoryCompressorException(
                fmt::format("Entry's size exceeds remaining data's size ({} > {})", entry_info.compressed_length,
                            left_size));
    }
    return entry_info;
}

std::streamsize DirEntryInfo::writeToStream(std::ofstream &stream) {
    stream.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::size_t path_len = relative_path.size();
    if (path_len > PATH_MAX_LEN) {
        throw DirectoryCompressorException(
                fmt::format("Cannot write path to stream (too long! {} > {})", path_len, PATH_MAX_LEN));
    }
    stream.write(std::bit_cast<char *>(&path_len), sizeof(path_len));
    stream.write(relative_path.c_str(), static_cast<std::streamsize>(path_len));
    if (is_directory) {
        compressed_length = 0;
        stream.write(std::bit_cast<char *>(&compressed_length), sizeof(compressed_length));
    }
    auto pos_to_write_compressed_size = stream.tellp();
    return pos_to_write_compressed_size;
}

void DirEntryInfo::writeCompressedLength(std::ofstream &stream, std::streamsize pos, std::size_t total_written) {
    stream.seekp(pos);
    stream.write(std::bit_cast<char *>(&total_written), sizeof(total_written));
    stream.seekp(0, std::ios::end);
    stream.flush();
    spdlog::info("Total written: {}", total_written);
}

size_t getLeftBytes(std::ifstream &zip_file, size_t total_file_length) {
    std::streamoff current_position = zip_file.tellg();

    if (zip_file.fail() || current_position < 0) {
        throw DirectoryCompressorException("Could not retrieve file's current position");
    }

    std::size_t left_size = total_file_length - static_cast<std::size_t>(current_position);
    return left_size;
}
