#pragma once

#include "SocketBase.hpp"

#include <thread>

class ClientSession : public SocketBase {
public:
    ClientSession(const std::string& host, unsigned short port);
    ClientSession(const std::string& host, unsigned short port, const std::string& path_to_cert_authority_file);
    ~ClientSession();
    void start();
private:
    ClientSession();


    bool verify_certificate(bool preverified,boost::asio::ssl::verify_context &ctx);
    void connect(const std::string& host, unsigned short port);

    boost::asio::io_context io_context{};
    boost::asio::ssl::context context{boost::asio::ssl::context::sslv23};
    std::jthread context_thread;
};