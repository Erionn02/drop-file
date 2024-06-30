#include "DropFileSendClient.hpp"
#include "InitSessionMessage.hpp"

#include <spdlog/spdlog.h>

DropFileSendClient::DropFileSendClient(ClientSocket socket) : socket(std::move(socket)) {}

void DropFileSendClient::sendFile(const std::string &path) {
    spdlog::debug("Client started.");
    nlohmann::json message_json = InitSessionMessage::createSendMessage(path);
    socket.SocketBase::send(message_json.dump());
    spdlog::info("Message sent.");
    std::string received_msg = socket.SocketBase::receive();
    spdlog::info("Server response: {}", received_msg);
    spdlog::info("Waiting for socket to confirm");
    socket.SocketBase::receiveACK();
    std::ifstream file{path};
    sendFileImpl(std::move(file));
}

void DropFileSendClient::sendFileImpl(std::ifstream data_source) {
    std::streamsize bytes_read;
    auto [buffer_ptr, buffer_size] = socket.SocketBase::getBuffer();
    do {
        bytes_read = data_source.readsome(buffer_ptr, static_cast<std::streamsize>(buffer_size));
        spdlog::debug("Bytes read: {}", bytes_read);
        if (bytes_read > 0) {
            socket.SocketBase::send({buffer_ptr, static_cast<std::size_t>(bytes_read)});
            socket.SocketBase::receiveACK();
        }
    } while (bytes_read > 0);
}
