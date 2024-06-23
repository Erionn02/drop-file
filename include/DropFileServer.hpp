#pragma once

#include "SessionsManager.hpp"

#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;
using boost::system::error_code;
class SessionsManager;

class DropFileServer {
public:
    DropFileServer(unsigned short port);
    void run();
private:
    void do_accept();

    boost::asio::io_service io_service;
    tcp::acceptor acceptor_;
    boost::asio::ssl::context context_;
    std::shared_ptr<SessionsManager> session_manager{std::make_shared<SessionsManager>()};
public:
    static inline unsigned short DEFAULT_PORT{12345};
};
