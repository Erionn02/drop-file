#include "Utils.hpp"

#include <openssl/evp.h>
#include <fmt/format.h>

#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <memory>
#include <cmath>


std::string hashToHexString(const std::vector<unsigned char> &hash);

std::string calculateFileHash(const std::filesystem::path &path) {
    constexpr int buffer_size = 8192;
    std::vector<unsigned char> buffer(buffer_size);

    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> mdctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    const EVP_MD *md = EVP_sha256();

    if (!mdctx) {
        throw std::runtime_error("Error creating context for hashing.");
    }

    if (EVP_DigestInit_ex(mdctx.get(), md, nullptr) != 1) {
        throw std::runtime_error("Error initializing digest context.");
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Error opening file: " + path.string());
    }

    while (file.good()) {
        file.read(reinterpret_cast<char *>(buffer.data()), buffer_size);
        if (file.gcount() > 0) {
            if (EVP_DigestUpdate(mdctx.get(), buffer.data(), static_cast<size_t>(file.gcount())) != 1) {
                throw std::runtime_error("Error updating digest.");
            }
        }
    }

    std::vector<unsigned char> hash_result(static_cast<unsigned long>(EVP_MD_size(md)));
    if (EVP_DigestFinal_ex(mdctx.get(), hash_result.data(), nullptr) != 1) {
        throw std::runtime_error("Error finalizing digest.");
    }

    return hashToHexString(hash_result);
}

std::string hashToHexString(const std::vector<unsigned char> &hash) {
    std::ostringstream hexStr;
    for (unsigned char byte: hash) {
        hexStr << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return hexStr.str();
}

std::string bytesToHumanReadable(std::size_t bytes) {
    constexpr std::size_t step{10};
    auto order_of_magnitude = static_cast<std::size_t>(std::log2(bytes)) / step;
    switch (order_of_magnitude) {
        case 0:
            return fmt::format("{} B", bytes);
        case 1:
            return fmt::format("{} kiB", bytes / 1024);
        case 2:
            return fmt::format("{} MiB", bytes / (1024 * 1024));
        case 3:
        default:
            return fmt::format("{} GiB", bytes / (1024ull * 1024ull * 1024ull));
    }
}

std::size_t getDirectorySize(const std::filesystem::path &directory) {
    constexpr std::size_t single_dir_disk_usage{4096};

    std::size_t total_size{single_dir_disk_usage};

    for (const auto &dir_entry: std::filesystem::recursive_directory_iterator(directory)) {
        if(dir_entry.is_directory()) {
            total_size += single_dir_disk_usage;
        } else if (dir_entry.is_regular_file()) {
            total_size += dir_entry.file_size();
        }
    }

    return total_size;
}
