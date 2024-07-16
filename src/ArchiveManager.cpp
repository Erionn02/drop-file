#include "ArchiveManager.hpp"
#include "Utils.hpp"
#include "gzip.hpp"

#include <fmt/format.h>


ArchiveManager::ArchiveManager(fs::path directory) : directory(std::move(directory)),
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


void ArchiveManager::unpackArchive(const fs::path &archive_path) {
    if (!fs::exists(directory)) {
        throw ArchiveManagerException(
                fmt::format("Directory that you try to unpack archive into ({}) does not exists!",
                            archive_path.string()));
    }
    if (!fs::exists(archive_path)) {
        throw ArchiveManagerException(
                fmt::format("Given archive {} does not exists!", archive_path.string()));
    }
    std::ifstream compressed_archive(archive_path, std::ios_base::binary);
    std::size_t archive_size = std::filesystem::file_size(archive_path);

    progress_bar.set_option(indicators::option::PrefixText{"Unpacking... "});
    while (getRemainingBytes(compressed_archive, archive_size) > 0) {
        FSEntryInfo entry_info = FSEntryInfo::readFromStream(compressed_archive, archive_size);
        progress_bar.tick();
        if (entry_info.is_directory) {
            std::filesystem::create_directories(directory / entry_info.relative_path);
        } else {
            unpackFile(compressed_archive, entry_info);
        }
    }

    progress_bar.set_option(indicators::option::PrefixText{"Unpacked. "});
    progress_bar.mark_as_completed();
}

void ArchiveManager::unpackFile(std::ifstream &compressed_archive, const FSEntryInfo &entry_info) {
    std::ofstream decompressed_file{directory / entry_info.relative_path, std::ios::binary};

    if (!decompressed_file.is_open()) {
        throw ArchiveManagerException(
                "Failed to open output decompressed_file: " + (directory / entry_info.relative_path).string());
    }
    gzip::decompress(decompressed_file, compressed_archive, entry_info.compressed_length);
}

void ArchiveManager::createArchive(const fs::path &new_archive_path) {
    if (fs::exists(new_archive_path)) {
        throw ArchiveManagerException(
                fmt::format("File that you try to unpackArchive to ({}) already exists!",
                            new_archive_path.string()));
    }
    std::ofstream new_archive(new_archive_path, std::ios::binary | std::ios::trunc);
    progress_bar.set_option(indicators::option::PrefixText{"Building archive... "});

    packDirectory(directory, new_archive, directory.filename());

    progress_bar.set_option(indicators::option::PrefixText{"Archive built. "});
    progress_bar.mark_as_completed();
}

void ArchiveManager::packDirectory(const fs::path &dir_to_compress, std::ofstream &new_archive,
                                   const fs::path &relative_path) {
    addDirectory(new_archive, relative_path);
    for (const auto &dir_entry: fs::directory_iterator(dir_to_compress)) {
        progress_bar.tick();
        const fs::path &current = dir_entry.path();
        fs::path new_relative = relative_path / current.filename();

        if (fs::is_directory(current)) {
            packDirectory(current, new_archive, new_relative);
        } else if (fs::is_regular_file(current)) {
            addFile(current, new_archive, new_relative);
        }
    }
}

void ArchiveManager::addDirectory(std::ofstream &new_archive, const fs::path &relative_path) {
    FSEntryInfo info{true, relative_path};
    info.writeToStream(new_archive);
}

void ArchiveManager::addFile(const fs::path &file_path, std::ofstream &compressed_archive,
                             const fs::path &relative_path) {
    FSEntryInfo file_info{false, relative_path};
    auto pos_to_write_compressed_size = file_info.writeToStream(compressed_archive);
    std::ifstream input_file{file_path, std::ios::binary};
    if (!input_file.is_open()) {
        throw ArchiveManagerException("Failed to open input file: " + file_path.string());
    }
    std::size_t bytes_written = gzip::compress(input_file, compressed_archive, [&] {
        progress_bar.tick();
    });

    file_info.writeCompressedLength(compressed_archive, pos_to_write_compressed_size, bytes_written);
}