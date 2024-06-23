#include "ServerSideClientSession.hpp"

#include "InitSessionMessage.hpp"

#include <iostream>


ServerSideClientSession::ServerSideClientSession(tcp::socket socket, asio::ssl::context &context)
        : SocketBase({std::move(socket), context}) {
}

void ServerSideClientSession::start() {
    socket_.handshake(boost::asio::ssl::stream_base::server);
    socket_.async_read_some(asio::buffer(data_), callback(&ServerSideClientSession::handleFirstRead));
}

void ServerSideClientSession::disconnect(std::optional<std::string> disconnect_msg) {
    if(disconnect_msg.has_value()) {
        socket_.async_write_some(asio::buffer(*disconnect_msg), callback(&ServerSideClientSession::disconnectWriteCallback));
    } else {
        socket_.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_send);
    }
}

void ServerSideClientSession::disconnectWriteCallback(error_code, size_t) {
    socket_.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_send);
}

void ServerSideClientSession::handleFirstRead(error_code error, size_t bytes_transferred)  {
    if (!error) {
        std::string_view content {data_.data(), bytes_transferred};
        std::cout <<  content << std::endl;
        try {
            nlohmann::json json = InitSessionMessage::create(content);
            if (json[InitSessionMessage::ACTION_KEY] == "send") {
                // handle send request
            } else {
                // handle receive request
            }
            socket_.async_write_some(asio::buffer(std::string("Gotcha json buddy")), callback(
                    &ServerSideClientSession::handleFistWrite));
            socket_.async_read_some(asio::buffer(data_), callback(&ServerSideClientSession::handleRead));
        } catch (const InitSessionMessageException& e) {
            disconnect(e.what());
        }

    }
}

void ServerSideClientSession::handleRead(error_code error, size_t bytes_transferred) {
    if (!error) {
        std::cout << std::string_view(data_.data(), bytes_transferred) << std::endl;
        socket_.async_write_some(asio::buffer(std::string("ok")), callback(&ServerSideClientSession::handleWrite));
    }
}

void ServerSideClientSession::handleWrite(error_code ec, std::size_t bytes_transferred) {
    if (!ec) {
        std::cout << std::string_view(data_.data(), bytes_transferred) << std::endl;
        socket_.async_read_some(asio::buffer(data_), callback(&ServerSideClientSession::handleRead));
    }
}

void ServerSideClientSession::handleFistWrite(error_code, size_t) {

}
