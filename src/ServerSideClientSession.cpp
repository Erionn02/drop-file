#include "ServerSideClientSession.hpp"
#include "InitSessionMessage.hpp"
#include "SessionsManager.hpp"

#include <spdlog/spdlog.h>



ServerSideClientSession::ServerSideClientSession(tcp::socket socket, asio::ssl::context &context, std::weak_ptr<SessionsManager> sessions_manager)
        : SocketBase({std::move(socket), context}), sessions_manager(std::move(sessions_manager)) {
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
        spdlog::debug(content);
        try {
            nlohmann::json json = InitSessionMessage::create(content);
            registerSession(json);
        } catch (const InitSessionMessageException& e) {
            disconnect(e.what());
        } catch (const SessionsManagerException& e) {
            disconnect(e.what());
        }
    }
}

void ServerSideClientSession::registerSession(const nlohmann::json &json) {
    if (auto manager = sessions_manager.lock()) {
        if (json[InitSessionMessage::ACTION_KEY] == "send") {
            std::string session_code = manager->registerSession(shared_from_this());
            socket_.write_some(asio::buffer(fmt::format("Enter on another device: 'drop-file receive {}'", session_code)));
        } else {
            std::string code_words_key = json[InitSessionMessage::CODE_WORDS_KEY];
            manager->addReceiver(code_words_key, shared_from_this());
        }
        socket_.async_read_some(asio::buffer(data_), callback(&ServerSideClientSession::handleRead));
    } else {
        disconnect("Internal error"); // should not ever happen
    }
}

void ServerSideClientSession::handleRead(error_code error, size_t bytes_transferred) {
    if (!error) {
        spdlog::info(std::string_view(data_.data(), bytes_transferred));
        socket_.async_write_some(asio::buffer(std::string("ok")), callback(&ServerSideClientSession::handleWrite));
    }
}

void ServerSideClientSession::handleWrite(error_code ec, std::size_t bytes_transferred) {
    if (!ec) {
        spdlog::info(std::string_view(data_.data(), bytes_transferred));
        socket_.async_read_some(asio::buffer(data_), callback(&ServerSideClientSession::handleRead));
    }
}
