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

class ServerSideClientSession: public SocketBase, public std::enable_shared_from_this<ServerSideClientSession> {
public:
    ServerSideClientSession(tcp::socket socket, boost::asio::ssl::context &context, std::weak_ptr<SessionsManager> sessions_manager);
    ~ServerSideClientSession();
    void start();
    void disconnect(std::optional<std::string> disconnect_msg);
private:
    template <typename PMF> auto callback(PMF pmf) { return std::bind(pmf, shared_from_this(), _1, _2); }
    void registerSession(nlohmann::json json);
    std::string_view extractContent(size_t bytes_transferred) const;
    void handleFirstRead(error_code error, size_t bytes_transferred);
    void receiveFile(std::shared_ptr<ServerSideClientSession> sender, nlohmann::json session_metadata);


    std::weak_ptr<SessionsManager> sessions_manager;
    static constexpr std::size_t MAX_FIRST_MESSAGE_SIZE{1000};
};
