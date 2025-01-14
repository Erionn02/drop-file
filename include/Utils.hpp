#pragma once
#include "DropFileBaseException.hpp"

#include <stdint.h>
#include <indicators/progress_bar.hpp>

#include <filesystem>
#include <string>


class UtilsException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};

std::string calculateFileHash(const std::filesystem::path& path);

std::string bytesToHumanReadable(std::size_t bytes);

indicators::ProgressBar createProgressBar(const std::string& initial_text);

size_t getRemainingBytes(std::istream &zip_file, size_t total_stream_length);

std::string pickRandom(const std::vector<std::string>& elements);

std::size_t getRandom(std::size_t a, std::size_t b);