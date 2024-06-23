#pragma once

#include "SocketBase.hpp"


class ClientSession : public SocketBase {
public:
    ClientSession(boost::asio::io_context &io_context,
                  boost::asio::ssl::context &context,
                  const tcp::resolver::results_type &endpoints);

private:
    bool verify_certificate(bool preverified,boost::asio::ssl::verify_context &ctx);
    void connect(const tcp::resolver::results_type &endpoints);
};