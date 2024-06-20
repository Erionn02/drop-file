#include "SocketBase.hpp"

SocketBase::SocketBase(boost::asio::ssl::stream<tcp::socket> socket_): socket_(std::move(socket_)) {

}

void SocketBase::send(const std::string &data) {
    MSG_HEADER_t message_length = data.size();
    assert(message_length<=MAX_MSG_SIZE); //todo
    std::vector<boost::asio::const_buffer> message{};
    message.emplace_back(boost::asio::buffer(&message_length, sizeof(message_length)));
    message.push_back(boost::asio::buffer(data));
    socket_.write_some(message);
}

std::string SocketBase::receive() {
    MSG_HEADER_t message_length{};
    boost::asio::mutable_buffer buffer{&message_length, sizeof(message_length)};
    boost::asio::read(socket_, buffer, boost::asio::transfer_exactly(buffer.size()));
    assert(message_length<=MAX_MSG_SIZE); //todo
    std::string message{};
    message.resize(message_length);
    boost::asio::read(socket_, boost::asio::buffer(message), boost::asio::transfer_exactly(message_length));
    return message;
}
