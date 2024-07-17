#pragma once

#include <string>
#include <optional>

enum class Action {
    send,
    receive
};

struct ClientArgs {
    Action action;
    std::optional<std::string> file_to_send_path{std::nullopt};
    std::optional<std::string> receive_code{std::nullopt};
    unsigned short port;
    std::string server_domain_name;
    bool verify_cert;
};