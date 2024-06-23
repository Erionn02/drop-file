#pragma once

#include "SocketBase.hpp"

#include <nlohmann/json.hpp>

#include <memory>


using namespace std::placeholders;
namespace asio = boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

class SessionsManager;

class ServerSideClientSession: public SocketBase, public std::enable_shared_from_this<ServerSideClientSession> {
public:
    ServerSideClientSession(tcp::socket socket, boost::asio::ssl::context &context, std::weak_ptr<SessionsManager> sessions_manager);

    void start();
    void disconnect(std::optional<std::string> disconnect_msg);
private:
    template <typename PMF> auto callback(PMF pmf) { return std::bind(pmf, shared_from_this(), _1, _2); }
    void registerSession(const nlohmann::json &json);
    void disconnectWriteCallback(error_code, size_t);
    void handleFirstRead(error_code error, size_t bytes_transferred);
    void handleRead(error_code error, size_t bytes_transferred);
    void handleWrite(error_code ec, std::size_t bytes_transferred);

    std::weak_ptr<SessionsManager> sessions_manager;
};
