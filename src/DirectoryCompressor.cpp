#include "DirectoryCompressor.hpp"
#include "Utils.hpp"

#include <zlib.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>


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
        throw DirectoryCompressorException(fmt::format("Given archive {} does not exists!", compressed_file_path.string()));
    }
    std::ifstream ifs(compressed_file_path, std::ios_base::binary);
    std::size_t file_size = std::filesystem::file_size(compressed_file_path);

    progress_bar.set_option(indicators::option::PrefixText{"Decompressing..."});
    while (getLeftBytes(ifs, file_size) > 0) {
        DirEntryInfo entry_info = readDirEntryInfo(ifs, file_size);
        spdlog::info("Path: {}, compressed size: {}", entry_info.relative_path, entry_info.compressed_length);
        if (entry_info.is_directory) {
            std::filesystem::create_directories(directory / entry_info.relative_path);
        } else {
            decompressFile(ifs, entry_info);
        }
    }

    progress_bar.set_option(indicators::option::PrefixText{"Decompressed."});
    progress_bar.mark_as_completed();
}


DirEntryInfo DirectoryCompressor::readDirEntryInfo(std::ifstream &zip_file, std::size_t total_file_length) {
    DirEntryInfo entry_info;
    auto bytes_read = zip_file.readsome(std::bit_cast<char *>(&entry_info), sizeof(entry_info));
    if (bytes_read != sizeof(entry_info)) {
        throw DirectoryCompressorException(fmt::format("Could not read dir entry info. ({}!={})", bytes_read, sizeof(entry_info)));
    }
    size_t left_size = getLeftBytes(zip_file, total_file_length);

    if (entry_info.compressed_length > left_size) {
        throw DirectoryCompressorException(
                fmt::format("Entry's size exceeds remaining data's size ({} > {})", entry_info.compressed_length,
                            left_size));
    }
    return entry_info;
}

size_t DirectoryCompressor::getLeftBytes(std::ifstream &zip_file, size_t total_file_length) const {
    std::streamoff current_position = zip_file.tellg();

    if (zip_file.fail() || current_position < 0) {
        throw DirectoryCompressorException("Could not retrieve file's current position");
    }

    std::size_t left_size = total_file_length - static_cast<std::size_t>(current_position);
    return left_size;
}

void DirectoryCompressor::decompressFile(std::ifstream &compressed_file, const DirEntryInfo &entry_info) {
    std::ofstream file{directory / entry_info.relative_path, std::ios::binary};

    if (!file.is_open()) {
        throw DirectoryCompressorException(
                "Failed to open output file: " + (directory / entry_info.relative_path).string());
    }

    std::array<char, BUFFER_SIZE> input_buffer{};
    std::array<unsigned char, BUFFER_SIZE> decompression_buffer{};
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    inflateInit(&zs);
    std::unique_ptr<z_stream, decltype(&inflateEnd)> stream_guard{&zs, inflateEnd};


    zs.next_out = decompression_buffer.data();
    zs.avail_out = decompression_buffer.size();

    std::size_t total_bytes_read{0};
    std::size_t bytes_left = entry_info.compressed_length - total_bytes_read;
    std::size_t this_chunk_size{std::min(BUFFER_SIZE, bytes_left)};
    while (bytes_left > 0 && compressed_file.read(input_buffer.data(), static_cast<std::streamsize>(this_chunk_size))) {
        total_bytes_read += static_cast<size_t>(compressed_file.gcount());
        bytes_left = entry_info.compressed_length - total_bytes_read;
        this_chunk_size = std::min(BUFFER_SIZE, bytes_left);

        zs.next_in = reinterpret_cast<unsigned char *>(input_buffer.data());
        zs.avail_in = static_cast<unsigned int>(compressed_file.gcount());

        while (zs.avail_in > 0) {
            inflate(&zs, Z_NO_FLUSH);
            file.write(reinterpret_cast<char *>(decompression_buffer.data()), BUFFER_SIZE - zs.avail_out);
            zs.next_out = decompression_buffer.data();
            zs.avail_out = BUFFER_SIZE;
        }
    }

    int ret = Z_OK;
    while (ret != Z_STREAM_END) {
        ret = inflate(&zs, Z_FINISH);
        file.write(reinterpret_cast<char *>(decompression_buffer.data()), BUFFER_SIZE - zs.avail_out);
        zs.next_out = decompression_buffer.data();
        zs.avail_out = BUFFER_SIZE;
    }
}

void DirectoryCompressor::compress(const fs::path &new_compressed_file_path) {
    if (fs::exists(new_compressed_file_path)) {
        throw DirectoryCompressorException(
                fmt::format("File that you try to decompress to ({}) already exists!",
                            new_compressed_file_path.string()));
    }
    std::ofstream outFile(new_compressed_file_path, std::ios::binary);
    progress_bar.set_option(indicators::option::PrefixText{"Compressing directory..."});

    compressDirectory(directory, outFile, directory.filename());

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
            compressFile(current, compressed_archive, relative_path);
        }
    }
}

void DirectoryCompressor::addDirectory(std::ofstream &out_file, const fs::path &relative_path) {
    DirEntryInfo info{true, relative_path};
    out_file.write(std::bit_cast<const char *>(&info), sizeof(info));
}

void DirectoryCompressor::compressFile(const fs::path &file_path, std::ofstream &compressed_archive,
                                       const fs::path &relative_path) {
    std::ifstream input_file(file_path, std::ios::binary);
    if (!input_file.is_open()) {
        throw DirectoryCompressorException("Failed to open input file: " + file_path.string());
    }
    DirEntryInfo file_info{false, relative_path};
    compressed_archive << file_info.is_directory;
    compressed_archive.write(file_info.relative_path, static_cast<std::streamsize>(DirEntryInfo::PATH_MAX_LEN));
    auto pos_to_write_compressed_size = compressed_archive.tellp();


    std::array<char, BUFFER_SIZE> input_buffer{};
    std::array<unsigned char, BUFFER_SIZE> compression_buffer{};

    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    deflateInit(&zs, Z_BEST_SPEED);
    std::unique_ptr<z_stream, decltype(&deflateEnd)> stream_guard{&zs, deflateEnd};

    std::size_t total_written{0};
    while (input_file.read(input_buffer.data(), BUFFER_SIZE)) {
        spdlog::info("Read {} bytes.", input_file.gcount());
        zs.next_in = reinterpret_cast<unsigned char *>(input_buffer.data());
        zs.avail_in = static_cast<unsigned int>(input_file.gcount());

        zs.next_out = compression_buffer.data();
        zs.avail_out = BUFFER_SIZE;

        while (zs.avail_in > 0) {
            spdlog::info("zs.avail_in: {}", zs.avail_in);
            auto [_, bytes_written] = compressChunk(compressed_archive, zs, compression_buffer, Z_NO_FLUSH);
            total_written += bytes_written;
        }
    }

    zs.next_in = reinterpret_cast<unsigned char *>(input_buffer.data());
    zs.avail_in = 0;
    int ret = Z_OK;
    while (ret != Z_STREAM_END) {
        auto [ret_code, bytes_written] = compressChunk(compressed_archive, zs, compression_buffer, Z_FINISH);
        ret = ret_code;
        total_written += bytes_written;
    }

    compressed_archive.seekp(pos_to_write_compressed_size);
    compressed_archive << total_written;
    compressed_archive.seekp(0, std::ios::end);
}

std::pair<int, std::size_t> DirectoryCompressor::compressChunk(std::ofstream &compressed_file, z_stream &zs,
                                                               std::array<unsigned char, BUFFER_SIZE> &compression_buffer, int flush) {
    progress_bar.tick();
    int ret = deflate(&zs, flush);
    std::size_t bytes_written = compression_buffer.size() - zs.avail_out;
    compressed_file.write(reinterpret_cast<char *>(compression_buffer.data()),
                          static_cast<std::streamsize>(bytes_written));
    zs.next_out = compression_buffer.data();
    zs.avail_out = static_cast<unsigned int>(compression_buffer.size());
    return {ret, bytes_written};
}

DirEntryInfo::DirEntryInfo(bool is_directory, const fs::path &relative_path) : is_directory(
        is_directory) {
    auto path_str = relative_path.string();
    if (path_str.size() > DirEntryInfo::PATH_MAX_LEN) {
        throw DirectoryCompressorException(
                fmt::format("{} path is longer than allowed ({}>{})", path_str, path_str.size(),
                            DirEntryInfo::PATH_MAX_LEN));
    }
    std::memcpy(this->relative_path, path_str.data(), path_str.size());
}

//void
//DirectoryCompressor::checkLeftDiskSpace(const fs::path &zip_file_path) const {
//    std::size_t original_size{2137};
//    auto left_disk_space = std::filesystem::space(directory.parent_path()).free;
//    if (left_disk_space < original_size) {
//        throw DirectoryCompressorException(fmt::format("Not enough space to decompress {}.", zip_file_path.string()));
//    }
//}
