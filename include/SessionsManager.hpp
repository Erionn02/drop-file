#pragma once

#include <memory>
#include <atomic>


class ServerSideClientSession;


class SessionsManager {
public:
    void add(std::shared_ptr<ServerSideClientSession> session);

    std::atomic_size_t connections{0};
};