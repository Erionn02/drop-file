#include "SessionsManager.hpp"

#include <ServerSideClientSession.hpp>


std::string SessionsManager::registerSession(std::shared_ptr<ServerSideClientSession> sender) {
    (void)sender;
    return generateSessionID();
}

void SessionsManager::addReceiver(const std::string &session_code, std::shared_ptr<ServerSideClientSession> receiver) {
    (void)session_code;
    (void)receiver;
}

std::string SessionsManager::generateSessionID() {
    return {"unique-session-id"};
}
