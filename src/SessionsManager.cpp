#include "SessionsManager.hpp"
#include "ServerSideClientSession.hpp"
#include "InitSessionMessage.hpp"
#include "Utils.hpp"
#include "JsonWords.hpp"

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <random>
#include <fstream>
#include <filesystem>

SessionsManager::SessionsManager() : SessionsManager(DEFAULT_CLIENT_TIMEOUT,
                                                     DEFAULT_CHECK_INTERVAL) {}

SessionsManager::SessionsManager(std::chrono::seconds client_timeout,
                                 std::chrono::seconds check_interval) : nouns(extractWords(WORDS_JSON, "nouns")),
                                                                        adjectives(extractWords(WORDS_JSON,
                                                                                                "adjectives")) {
    connections_controller = std::jthread{[client_timeout, check_interval, this](const std::stop_token &stop_token) {
        while (!stop_token.stop_requested()) {
            terminateTimeoutClients(client_timeout);
            std::this_thread::sleep_for(check_interval);
        }
    }};
}

void SessionsManager::terminateTimeoutClients(std::chrono::seconds client_timeout) {
    std::unique_lock lock{m};
    for (auto it = senders_sessions.begin(); it != senders_sessions.end();) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed_time = (now - it->second.time_point);
        if (elapsed_time > client_timeout) {
            spdlog::info("Terminating sender client with code: '{}'", it->first);
            it = senders_sessions.erase(it);
        } else {
            ++it;
        }
    }
}

std::string SessionsManager::registerSender(std::shared_ptr<ServerSideClientSession> sender, nlohmann::json json) {
    while (true) {
        auto session_id = generateSessionID();
        std::unique_lock lock{m};
        if (senders_sessions.contains(session_id)) {
            continue;
        }
        senders_sessions[session_id] = {.client_session = std::move(sender), .session_data = std::move(json),
                .time_point = std::chrono::high_resolution_clock::now()};
        return session_id;
    }
}

std::pair<std::shared_ptr<ServerSideClientSession>, nlohmann::json>
SessionsManager::getSenderWithMetadata(const std::string &session_code) {
    std::unique_lock lock{m};
    auto it = senders_sessions.find(session_code);
    if (it == senders_sessions.end()) {
        throw SessionsManagerException{"Unknown session code."};
    }
    auto node = senders_sessions.extract(it);
    return {std::move(node.mapped().client_session), std::move(node.mapped().session_data)};
}

std::string SessionsManager::generateSessionID() {
    return fmt::format("{}-{}-{}", pickRandom(adjectives), pickRandom(nouns), getRandom(0, 100));
}

std::size_t SessionsManager::currentSessions() {
    std::unique_lock lock{m};
    return senders_sessions.size();
}
