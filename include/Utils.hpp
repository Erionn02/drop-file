#pragma once

#include <indicators/progress_bar.hpp>

#include <filesystem>
#include <string>


std::string calculateFileHash(const std::filesystem::path& path);

std::string bytesToHumanReadable(std::size_t bytes);

std::size_t getDirectorySize(const std::filesystem::path& directory);

indicators::ProgressBar createProgressBar(const std::string& initial_text);