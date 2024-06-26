#include "SessionsManager.hpp"
#include "ServerSideClientSession.hpp"

#include <random>


std::string SessionsManager::registerSender(std::shared_ptr<ServerSideClientSession> sender, nlohmann::json json) {
    std::unique_lock lock{m};
    auto session_id = generateSessionID(SESSION_ID_LENGTH);
    senders_sessions[session_id] = {std::move(sender), std::move(json)};
    return session_id;
}

std::pair<std::shared_ptr<ServerSideClientSession>, nlohmann::json> SessionsManager::getSenderWithMetadata(const std::string &session_code) {
    std::unique_lock lock{m};
    auto it = senders_sessions.find(session_code);
    if (it == senders_sessions.end()) {
        throw SessionsManagerException{"Unknown session code."};
    }
    auto node = senders_sessions.extract(it);
    std::pair<std::shared_ptr<ServerSideClientSession>, nlohmann::json> client_and_metadata = std::move(node.mapped());
    return client_and_metadata;
}

std::string SessionsManager::generateSessionID(std::size_t length) {
    static auto &chrs = "0123456789"
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
