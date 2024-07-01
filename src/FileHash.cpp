#include "FileHash.hpp"

#include <openssl/evp.h>

#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <memory>
#include <iomanip>
#include <stdexcept>


std::string hashToHexString(const std::vector<unsigned char>& hash);

std::string calculateFileHash(const std::filesystem::path& path) {
    constexpr int bufferSize = 8192;
    std::vector<unsigned char> buffer(bufferSize);

    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> mdctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    const EVP_MD* md = EVP_sha256();

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
        file.read(reinterpret_cast<char*>(buffer.data()), bufferSize);
        if (file.gcount() > 0) {
            if (EVP_DigestUpdate(mdctx.get(), buffer.data(), static_cast<size_t>(file.gcount())) != 1) {
                throw std::runtime_error("Error updating digest.");
            }
        }
    }

    std::vector<unsigned char> hashResult(static_cast<unsigned long>(EVP_MD_size(md)));
    if (EVP_DigestFinal_ex(mdctx.get(), hashResult.data(), nullptr) != 1) {
        throw std::runtime_error("Error finalizing digest.");
    }

    return hashToHexString(hashResult);
}

std::string hashToHexString(const std::vector<unsigned char>& hash) {
    std::ostringstream hexStr;
    for (unsigned char byte : hash) {
        hexStr << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return hexStr.str();
}

