#pragma once

#include <gtest/gtest.h>
#include <boost/range/combine.hpp>

#include <filesystem>
#include <random>
#include <fstream>


namespace fs = std::filesystem;

// normal iterating over directory does not guarantee any particular order of entries,
// therefore I have to sort it first
inline std::set<fs::directory_entry> getSortedDirEntries(const fs::path& dir) {
    return {fs::directory_iterator(dir), fs::directory_iterator()};
}

inline std::string getFileContent(const fs::path& file_path) {
    std::ifstream file{file_path, std::ios::binary};
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

inline bool filesContentEqual(const fs::path& p1, const fs::path& p2) {
    if (std::filesystem::file_size(p1) != std::filesystem::file_size(p2)) {
        return false;
    }
    std::ifstream file1{p1, std::ios::binary};
    std::ifstream file2{p2, std::ios::binary};
    constexpr std::size_t BUFFER_SIZE{10000};
    std::array<char, BUFFER_SIZE> buf1{};
    std::array<char, BUFFER_SIZE> buf2{};
    auto size = std::filesystem::file_size(p1);
    for(std::size_t total_read_each{0}; total_read_each < size; ) {
        file1.read(buf1.data(), BUFFER_SIZE);
        file2.read(buf2.data(), BUFFER_SIZE);
        std::string_view content1{buf1.data(), static_cast<std::size_t>(file1.gcount())};
        std::string_view content2{buf2.data(), static_cast<std::size_t>(file2.gcount())};
        if(content1!=content2) {
            return false;
        }
        total_read_each += file1.gcount();
    }
    return true;
}

inline void assertDirectoriesEqual(const fs::path& dir1, const fs::path& dir2) {
    auto dir_entries_1 = getSortedDirEntries(dir1);
    auto dir_entries_2 = getSortedDirEntries(dir2);
    ASSERT_EQ(dir_entries_1.size(), dir_entries_2.size());
    for(auto [el1, el2]: boost::range::combine(dir_entries_1, dir_entries_2)) { // std::views::zip requires gcc 13 :(
        ASSERT_EQ(el1.path().filename(), el2.path().filename());
        ASSERT_EQ(el1.is_directory(), el2.is_directory());
        ASSERT_EQ(el1.is_regular_file(), el2.is_regular_file());
        if (el1.is_directory()) {
            assertDirectoriesEqual(el1, el2);
        } else {
            ASSERT_EQ(std::filesystem::file_size(el1), std::filesystem::file_size(el2));
            ASSERT_TRUE(filesContentEqual(el1, el2));
        }
    }
}

inline std::string generateRandomString(std::size_t length) {
    static auto &chrs = "080886789"
                        "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    static thread_local std::mt19937 rg{std::random_device{}()};
    static thread_local std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string random_str;
    random_str.reserve(length);
    while (length--) {
        random_str += chrs[pick(rg)];
    }

    return random_str;
}