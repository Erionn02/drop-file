#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <stdexcept>

using boost::asio::ip::tcp;

class SocketException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class SocketBase {
public:
    SocketBase(boost::asio::ssl::stream<tcp::socket> socket_);
    void send(const std::string& data);
    bool isAnyMessageWaiting();
    std::string receive();
protected:
    void sendChunk(std::string_view data);

    boost::asio::ssl::stream<tcp::socket> socket_;
    using MSG_HEADER_t = std::size_t;
    std::array<char, 1024> data_;
    static constexpr MSG_HEADER_t MAX_MSG_SIZE{1024 * 1024 * 3}; // 3 MiB
};


