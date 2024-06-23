#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <memory>


class ServerSideClientSession;

class SessionsManagerException: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct FileExchangeSession {
    std::shared_ptr<ServerSideClientSession> sender;
    std::shared_ptr<ServerSideClientSession> receiver;
};

class SessionsManager {
public:
    std::string registerSession(std::shared_ptr<ServerSideClientSession> sender);
    void addReceiver(const std::string& session_code, std::shared_ptr<ServerSideClientSession> receiver);
private:
    std::string generateSessionID();


    std::mutex m;
    std::unordered_map<std::string, FileExchangeSession> sessions;
};