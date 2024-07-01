#pragma once

#include <filesystem>
#include <string>


std::string calculateFileHash(const std::filesystem::path& path);