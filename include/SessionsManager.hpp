#pragma once

#include "DropFileBaseException.hpp"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <memory>


class ServerSideClientSession;

class SessionsManagerException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};


class SessionsManager {
public:
    std::string registerSender(std::shared_ptr<ServerSideClientSession> sender);
    std::shared_ptr<ServerSideClientSession> getSender(const std::string& session_code);
private:
    std::string generateSessionID(std::size_t length);


    std::mutex m;
    std::unordered_map<std::string, std::shared_ptr<ServerSideClientSession>> senders_sessions;
    static constexpr std::size_t SESSION_ID_LENGTH{10};
};