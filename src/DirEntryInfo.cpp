#include "DirEntryInfo.hpp"
#include "Utils.hpp"

#include <fmt/format.h>


DirEntryInfo::DirEntryInfo(bool is_directory, const std::filesystem::path &relative_path) : is_directory(
        is_directory) {
    auto path_str = relative_path.string();
    if (path_str.size() > DirEntryInfo::PATH_MAX_LEN) {
        throw DirEntryInfoException(
                fmt::format("{} path is longer than allowed ({}>{})", path_str, path_str.size(),
                            DirEntryInfo::PATH_MAX_LEN));
    }
    this->relative_path = std::move(path_str);
}

DirEntryInfo DirEntryInfo::readFromStream(std::istream &stream, std::size_t total_file_length) {
    DirEntryInfo entry_info;
    stream.read(std::bit_cast<char *>(&entry_info.is_directory), sizeof(entry_info.is_directory));
    if (stream.gcount() != sizeof(entry_info.is_directory)) {
        throw DirEntryInfoException("Could read type of dir entry.");
    }
    std::size_t path_length{};
    stream.read(std::bit_cast<char *>(&path_length), sizeof(path_length));
    if (stream.gcount() != sizeof(path_length)) {
        throw DirEntryInfoException("Could read path length.");
    }
    if (path_length > PATH_MAX_LEN) {
        throw DirEntryInfoException(
                fmt::format("Path length exceeds maximum available value ({} > {})", path_length, PATH_MAX_LEN));
    }
    entry_info.relative_path.resize(path_length);
    stream.read(entry_info.relative_path.data(), static_cast<std::streamsize>(path_length));
    if (stream.gcount() != static_cast<std::streamsize>(path_length)) {
        throw DirEntryInfoException("Could read whole path.");
    }

    stream.read(std::bit_cast<char *>(&entry_info.compressed_length), sizeof(entry_info.compressed_length));
    if (stream.gcount() != sizeof(entry_info.compressed_length)) {
        throw DirEntryInfoException("Could read compressed length.");
    }

    size_t left_size = getRemainingBytes(stream, total_file_length);
    if (entry_info.compressed_length > left_size) {
        throw DirEntryInfoException(
                fmt::format("Entry's size exceeds remaining data's size ({} > {})", entry_info.compressed_length,
                            left_size));
    }
    return entry_info;
}

std::streamsize DirEntryInfo::writeToStream(std::ostream &stream) {
    stream.write(std::bit_cast<char *>(&is_directory), sizeof(is_directory));
    std::size_t path_len = relative_path.size();
    if (path_len > PATH_MAX_LEN) {
        throw DirEntryInfoException(
                fmt::format("Cannot write path to stream (too long! {} > {})", path_len, PATH_MAX_LEN));
    }
    stream.write(std::bit_cast<char *>(&path_len), sizeof(path_len));
    stream.write(relative_path.c_str(), static_cast<std::streamsize>(path_len));
    if (is_directory) {
        compressed_length = 0;
    }
    auto pos_to_write_compressed_size = stream.tellp();
    stream.write(std::bit_cast<char *>(&compressed_length), sizeof(compressed_length));
    return pos_to_write_compressed_size;
}

void DirEntryInfo::writeCompressedLength(std::ostream &stream, std::streamsize pos, std::size_t total_written) {
    stream.seekp(pos, std::ios_base::beg);
    stream.write(std::bit_cast<char *>(&total_written), sizeof(total_written));
    stream.seekp(0, std::ios::end);
    stream.flush();
}
