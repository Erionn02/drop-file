#pragma once
#include "DropFileBaseException.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <optional>


using boost::asio::ip::tcp;

class SocketException : public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};


// Every message is preceded by header that contains amount of bytes to send
class SocketBase : public std::enable_shared_from_this<SocketBase> {
public:
    SocketBase(boost::asio::ssl::stream<tcp::socket> socket_);
    virtual ~SocketBase() = default;

    using MessageHandler = std::function<void(std::string_view)>;
    void asyncReadMessage(std::size_t max_message_size, MessageHandler message_handler);
    void disconnect(std::optional<std::string> disconnect_msg);
    void connect(const std::string &host, unsigned short port);

    void send(std::string_view data);
    std::string receive();
    std::string_view receiveToBuffer();
protected:
    void sendChunk(std::string_view data);
    std::size_t getMessageLength() ;


    using MSG_HEADER_t = std::size_t;
    boost::asio::ssl::stream<tcp::socket> socket_;
    std::unique_ptr<char[]> data_buffer;
    static constexpr MSG_HEADER_t BUFFER_SIZE{1024 * 1024 * 1}; // 1 MiB
};


