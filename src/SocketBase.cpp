#include "SocketBase.hpp"

#include <fmt/format.h>

#include <spdlog/spdlog.h>

namespace asio = boost::asio;
using boost::system::error_code;

SocketBase::SocketBase(boost::asio::ssl::stream<tcp::socket> socket_) : socket_(std::move(socket_)),
                                                                        data_buffer(
                                                                                std::make_unique<char[]>(
                                                                                        BUFFER_SIZE)) {}

void SocketBase::send(std::string_view data) {
    spdlog::debug("SocketBase::send {}", data);
    MSG_HEADER_t message_length = data.size();
    MSG_HEADER_t ptr_cursor = 0;
    while (message_length >= BUFFER_SIZE) {
        sendChunk({data.data() + ptr_cursor, BUFFER_SIZE});
        message_length -= BUFFER_SIZE;
        ptr_cursor += BUFFER_SIZE;
    }
    if (message_length > 0) {
        sendChunk({data.data() + ptr_cursor, message_length});
    }
}

void SocketBase::sendChunk(std::string_view data) {
    MSG_HEADER_t message_length = data.size();
    std::vector<boost::asio::const_buffer> message{};
    message.emplace_back(boost::asio::buffer(&message_length, sizeof(message_length)));
    message.push_back(boost::asio::buffer(data));
    boost::asio::write(socket_, message);
}

void SocketBase::disconnect(std::optional<std::string> disconnect_msg) {
    spdlog::debug("Disconnecting... {}", disconnect_msg.value_or(""));
    if (disconnect_msg.has_value()) {
        socket_.write_some(asio::buffer(*disconnect_msg));
    }
    socket_.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_send);
}

std::string SocketBase::receive() {
    spdlog::debug("SocketBase::receive");
    MSG_HEADER_t message_length = getMessageLength();
    std::string message{};
    message.resize(message_length);
    boost::asio::read(socket_, boost::asio::buffer(message), boost::asio::transfer_exactly(message_length));
    return message;
}

std::size_t SocketBase::getMessageLength() {
    MSG_HEADER_t message_length{};
    boost::asio::mutable_buffer buffer{&message_length, sizeof(message_length)};
    boost::asio::read(socket_, buffer, boost::asio::transfer_exactly(buffer.size()));
    spdlog::debug("Receive message length: {}", message_length);
    if (message_length > BUFFER_SIZE) {
        throw SocketException(
                fmt::format("Somebody is trying to send {} bytes in single message, which is more than allowed ({})",
                            message_length, BUFFER_SIZE));
    }
    return message_length;
}

std::string_view SocketBase::receiveToBuffer() {
    MSG_HEADER_t message_length = getMessageLength();
    boost::asio::read(socket_, boost::asio::mutable_buffer(data_buffer.get(), message_length),
                      boost::asio::transfer_exactly(message_length));
    return {data_buffer.get(), message_length};
}

void SocketBase::asyncReadMessage(std::size_t max_msg_size, MessageHandler message_handler) {
    if (max_msg_size > BUFFER_SIZE) {
        throw SocketException(
                fmt::format("Tried to schedule receiving message with max size of {} bytes, where buffer size is {}.",
                            max_msg_size, BUFFER_SIZE));
    }
    asyncReadHeader(max_msg_size, std::move(message_handler));
}

void SocketBase::asyncReadHeader(size_t max_msg_size, SocketBase::MessageHandler message_handler) {
    constexpr std::size_t HEADER_SIZE = sizeof(MSG_HEADER_t);
    boost::asio::async_read(socket_, asio::buffer(data_buffer.get(), HEADER_SIZE),
                            asio::transfer_exactly(HEADER_SIZE),
                            [this, self = shared_from_this(), max_msg_size, message_handler = std::move(
                                    message_handler)](error_code ec, std::size_t) mutable {
                                if (!ec) {
                                    MSG_HEADER_t message_size = *std::bit_cast<MSG_HEADER_t *>(data_buffer.get());
                                    if (message_size > max_msg_size) {
                                        spdlog::info(
                                                "Somebody tried to send {} bytes, which is more than allowed ({}) for this callback.",
                                                message_size, max_msg_size);
                                        return;
                                    }
                                    asyncReadMessageImpl(std::move(self), std::move(message_handler), message_size);
                                } else {
                                    spdlog::debug("Encountered an error during async read, aborting. Details: {}",
                                                  ec.what());
                                }
                            });
}

void
SocketBase::asyncReadMessageImpl(std::shared_ptr<SocketBase> self, std::function<void(std::string_view)> message_handler,
                                 unsigned long message_size) {
    boost::asio::async_read(socket_, asio::buffer(data_buffer.get(), message_size),
                            asio::transfer_exactly(message_size),
                            [self = std::move(self), message_handler = std::move(
                                    message_handler), this](error_code ec,
                                                            std::size_t message_size) {
                                if (!ec) {
                                    message_handler(std::string_view{data_buffer.get(),
                                                                     message_size});
                                } else {
                                    spdlog::debug(
                                            "Encountered an error during async read, aborting. Details: {}",
                                            ec.what());
                                }
                            });
}

void SocketBase::receiveACK() {
    auto response = receiveToBuffer();
    if (response != SocketBase::ACK) {
        std::string_view additional_message;
        constexpr std::size_t MAX_PRINTABLE_STR_LENGTH{100}; // totally arbitrary number
        if (response.size() < MAX_PRINTABLE_STR_LENGTH) {
            additional_message = response;
        } else {
            additional_message = "Too large to print.";
        }
        throw SocketException(fmt::format("Response not ok. Response size: {}. {}", response.size(), additional_message));
    }
}

void SocketBase::sendACK() {
    send(SocketBase::ACK);
}

std::pair<char *, std::size_t> SocketBase::getBuffer() {
    return {data_buffer.get(), BUFFER_SIZE};
}
