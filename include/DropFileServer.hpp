#pragma once

#include "SessionsManager.hpp"

#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>

#include <filesystem>

using boost::asio::ip::tcp;
class SessionsManager;

class DropFileServer {
public:
    DropFileServer(unsigned short port, const std::filesystem::path& key_cert_dir);
    void run();
private:
    void acceptNewConnection();

    boost::asio::io_context io_context;
    tcp::acceptor acceptor_;
    boost::asio::ssl::context context_;
    std::shared_ptr<SessionsManager> session_manager{std::make_shared<SessionsManager>()};
public:
    static inline unsigned short DEFAULT_PORT{12345};
};
