#pragma once

#include <filesystem>

#include <spdlog/spdlog.h>


struct RAIIFSEntry {
    std::filesystem::path path;
    bool should_remove{false};

    RAIIFSEntry(std::filesystem::path path, bool should_remove) : path(std::move(path)), should_remove(should_remove) {}

    RAIIFSEntry(RAIIFSEntry &&other) noexcept {
        path = std::move(other.path);
        should_remove = other.should_remove;
        other.should_remove = false;
    }

    RAIIFSEntry& operator=(RAIIFSEntry &&other) noexcept {
        path = std::move(other.path);
        should_remove = other.should_remove;
        other.should_remove = false;
        return *this;
    }

    ~RAIIFSEntry() {
        if (should_remove) {
            spdlog::debug("Destroying");
            std::filesystem::remove_all(path);
        }
    }
};