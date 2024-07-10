#pragma once

#include <string>
#include <optional>
#include <chrono>

struct ServerArgs {
    std::string certs_directory{};
    unsigned short port{};
    std::chrono::seconds client_timeout{};

    static inline unsigned short DEFAULT_PORT{12345};
    static inline std::chrono::seconds DEFAULT_CLIENT_TIMEOUT{120};
};