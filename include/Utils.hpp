#pragma once

#include <filesystem>
#include <string>


std::string calculateFileHash(const std::filesystem::path& path);

std::string bytesToHumanReadable(std::size_t bytes);

std::size_t getDirectorySize(const std::filesystem::path& directory);