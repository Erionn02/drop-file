#include "ServerSideClientSession.hpp"
#include "InitSessionMessage.hpp"
#include "SessionsManager.hpp"

#include <spdlog/spdlog.h>



ServerSideClientSession::ServerSideClientSession(tcp::socket socket, asio::ssl::context &context, std::weak_ptr<SessionsManager> sessions_manager)
        : SocketBase({std::move(socket), context}), sessions_manager(std::move(sessions_manager)) {
}

ServerSideClientSession::~ServerSideClientSession() {
    spdlog::debug("ServerSideClientSession is being destroyed.");
}


void ServerSideClientSession::start() {
    socket_.handshake(boost::asio::ssl::stream_base::server);
    socket_.async_read_some(asio::buffer(data_buffer.get(), MAX_FIRST_MESSAGE_SIZE), callback(&ServerSideClientSession::handleFirstRead));
}

void ServerSideClientSession::disconnect(std::optional<std::string> disconnect_msg) {
    spdlog::debug("Disconnecting... {}", disconnect_msg.value_or(""));
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
        try {
            spdlog::debug("[ServerSideClientSession] Extracting content...");
            std::string_view content = extractContent(bytes_transferred);
            spdlog::debug("[ServerSideClientSession] Extracting json...");
            nlohmann::json json = InitSessionMessage::create(content);
            spdlog::debug("[ServerSideClientSession] Registering session...");
            registerSession(json);
            spdlog::debug("[ServerSideClientSession] Session registered.");
        } catch (const DropFileBaseException& e) {
            disconnect(e.what());
        }
    }
}

std::string_view ServerSideClientSession::extractContent(size_t bytes_transferred) const {
    if (bytes_transferred < sizeof(MSG_HEADER_t)) {
        throw ServerSideClientSessionException("Not enough bytes were transferred to read message header, aborting...");
    }
    MSG_HEADER_t message_bytes{*std::bit_cast<MSG_HEADER_t*>(data_buffer.get())};
    if (message_bytes > MAX_FIRST_MESSAGE_SIZE - sizeof(MSG_HEADER_t)) {
        throw ServerSideClientSessionException(fmt::format("Declared message size {} exceeds limit ({}), aborting...", message_bytes, MAX_FIRST_MESSAGE_SIZE));
    }
    std::size_t rest_of_message = bytes_transferred - sizeof(MSG_HEADER_t);
    if (rest_of_message != message_bytes) {
        throw ServerSideClientSessionException("Not enough bytes were transferred to read message, aborting...");
    }
    auto data_ptr = data_buffer.get() + sizeof(MSG_HEADER_t);
    std::string_view content {data_ptr, rest_of_message};
    spdlog::debug(content);
    return content;
}

void ServerSideClientSession::registerSession(const nlohmann::json &json) {
    if (auto manager = sessions_manager.lock()) {
        if (json[InitSessionMessage::ACTION_KEY] == "send") {
            std::string session_code = manager->registerSender(shared_from_this());
            send(fmt::format("Enter on another device: 'drop-file receive {}'", session_code));
        } else {
            std::string code_words_key = json[InitSessionMessage::CODE_WORDS_KEY];
            auto sender = manager->getSender(code_words_key);
            // todo handle data exchange
        }
    } else {
        disconnect("Internal error"); // should not ever happen
    }
}
