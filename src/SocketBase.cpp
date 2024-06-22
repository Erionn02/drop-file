#include "SocketBase.hpp"

#include <fmt/format.h>

SocketBase::SocketBase(boost::asio::ssl::stream<tcp::socket> socket_) : socket_(std::move(socket_)) {

}

void SocketBase::send(const std::string &data) {
    MSG_HEADER_t message_length = data.size();
    MSG_HEADER_t ptr_cursor = 0;
    while (message_length >= MAX_MSG_SIZE) {
        sendChunk({data.data() + ptr_cursor, MAX_MSG_SIZE});
        message_length -= MAX_MSG_SIZE;
        ptr_cursor += MAX_MSG_SIZE;
    }
    if (message_length > 0) {
        sendChunk({data.data() + ptr_cursor, message_length});
    }
}

void SocketBase::sendChunk(std::string_view data) {
    MSG_HEADER_t message_length = data.size();
    std::vector<boost::asio::const_buffer> message{};
//    message.emplace_back(boost::asio::buffer(&message_length, sizeof(message_length)));
    message.push_back(boost::asio::buffer(data));
    socket_.write_some(message);
}

std::string SocketBase::receive() {
//    MSG_HEADER_t message_length{};
//    boost::asio::mutable_buffer buffer{&message_length, sizeof(message_length)};
//    boost::asio::read(socket_, buffer, boost::asio::transfer_exactly(buffer.size()));
//    if (message_length > MAX_MSG_SIZE) {
//        throw SocketException(
//                fmt::format("Somebody is trying to send {} bytes in single message, which is more than allowed ({})",
//                            message_length, MAX_MSG_SIZE));
//    }
//    std::string message{};
//    message.resize(message_length);
//    boost::asio::read(socket_, boost::asio::buffer(message), boost::asio::transfer_exactly(message_length));
    auto bytes_read = socket_.read_some(boost::asio::mutable_buffer(data_.data(), data_.size()));
    return {data_.data(), bytes_read};
}

bool SocketBase::isAnyMessageWaiting() {
    return socket_.lowest_layer().available() >= sizeof(MSG_HEADER_t);
}
