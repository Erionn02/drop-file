#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

std::string sha256_file(const fs::path& path) {
    std::ifstream file(path.string(), std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path.string());
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    const std::size_t buffer_size = 1 << 12;
    char buffer[buffer_size];

    while (file.good()) {
        file.read(buffer, buffer_size);
        SHA256_Update(&sha256, buffer, file.gcount());
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    std::ostringstream result;
    for (unsigned char byte : hash) {
        result << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }

    return result.str();
}

std::string sha256_directory(const fs::path& path) {
    if (!fs::exists(path) || !fs::is_directory(path)) {
        throw std::runtime_error("Not a valid directory: " + path.string());
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    for (fs::recursive_directory_iterator it(path), end; it != end; ++it) {
        if (fs::is_regular_file(*it)) {
            std::string file_hash = sha256_file(*it);
            std::string relative_path = fs::relative(*it, path).string();

            SHA256_Update(&sha256, relative_path.c_str(), relative_path.size());
            SHA256_Update(&sha256, file_hash.c_str(), file_hash.size());
        } else if (fs::is_directory(*it)) {
            std::string relative_path = fs::relative(*it, path).string() + "/";
            SHA256_Update(&sha256, relative_path.c_str(), relative_path.size());
        }
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    std::ostringstream result;
    for (unsigned char byte : hash) {
        result << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }

    return result.str();
}

TEST(FileHasherTests, test1) {
    fs::path file_path = "file_to_hash.txt";
    fs::path dir_path = "directory_to_hash";

    try {
        std::string file_hash = sha256_file(file_path);
        std::cout << "SHA-256 hash of file " << file_path << ": " << file_hash << std::endl;

        std::string dir_hash = sha256_directory(dir_path);
        std::cout << "SHA-256 hash of directory " << dir_path << ": " << dir_hash << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
