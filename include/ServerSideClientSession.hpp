#pragma once

#include "SocketBase.hpp"
#include "DropFileBaseException.hpp"

#include <nlohmann/json.hpp>

#include <memory>

class ServerSideClientSessionException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};


using namespace std::placeholders;
namespace asio = boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

class SessionsManager;

class ServerSideClientSession: public SocketBase {
public:
    ServerSideClientSession(tcp::socket socket, boost::asio::ssl::context &context, std::weak_ptr<SessionsManager> sessions_manager);
    ~ServerSideClientSession();
    void start();
private:
    using PMF =  void (ServerSideClientSession::*)(std::string_view);
    MessageHandler callback(PMF pmf);

    void registerSession(nlohmann::json json);
    void handleFirstRead(std::string_view content);
    void receiveFile(std::shared_ptr<ServerSideClientSession> sender, nlohmann::json session_metadata);


    std::weak_ptr<SessionsManager> sessions_manager;
    static constexpr std::size_t MAX_FIRST_MESSAGE_SIZE{1000};
};
