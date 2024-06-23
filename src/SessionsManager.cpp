#include "SessionsManager.hpp"

#include <ServerSideClientSession.hpp>

void SessionsManager::add(std::shared_ptr<ServerSideClientSession> session) {
    session->start();
}
