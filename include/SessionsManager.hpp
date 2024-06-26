#pragma once

#include "DropFileBaseException.hpp"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>


class ServerSideClientSession;

class SessionsManagerException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};


class SessionsManager {
public:
    std::string registerSender(std::shared_ptr<ServerSideClientSession> sender,
                               nlohmann::json json);
    std::pair<std::shared_ptr<ServerSideClientSession>, nlohmann::json> getSenderWithMetadata(const std::string& session_code);
private:
    std::string generateSessionID(std::size_t length);


    std::mutex m;
    std::unordered_map<std::string, std::pair<std::shared_ptr<ServerSideClientSession>, nlohmann::json>> senders_sessions;
    static constexpr std::size_t SESSION_ID_LENGTH{10};
};