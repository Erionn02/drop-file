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


// Every message is preceded by header that contains amount of bytes to sendFile
class SocketBase : public std::enable_shared_from_this<SocketBase> {
public:
    SocketBase(boost::asio::ssl::stream<tcp::socket> socket_);
    SocketBase(SocketBase&&) = default;

    virtual ~SocketBase() = default;

    using MessageHandler = std::function<void(std::string_view)>;
    void asyncReadMessage(std::size_t max_message_size, MessageHandler message_handler);
    void disconnect(std::optional<std::string> disconnect_msg);

    void send(std::string_view data);
    std::string receive();
    std::string_view receiveToBuffer();

    void receiveACK();
    void sendACK();

    std::pair<char*, std::size_t> getBuffer();
protected:
    void sendChunk(std::string_view data);
    std::size_t getMessageLength() ;
    void asyncReadHeader(size_t max_msg_size, MessageHandler message_handler);
    void asyncReadMessageImpl(std::shared_ptr<SocketBase> self, std::function<void(std::string_view)> message_handler,
                              unsigned long message_size);


    using MSG_HEADER_t = std::size_t;
    boost::asio::ssl::stream<tcp::socket> socket_;
    std::unique_ptr<char[]> data_buffer;
    static constexpr MSG_HEADER_t BUFFER_SIZE{1024 * 1024 * 1}; // 1 MiB
    static inline const std::string ACK{"ACK"};
};


