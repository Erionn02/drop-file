#include "ServerSideClientSession.hpp"
#include "InitSessionMessage.hpp"
#include "SessionsManager.hpp"

#include <spdlog/spdlog.h>


ServerSideClientSession::ServerSideClientSession(tcp::socket socket, asio::ssl::context &context,
                                                 std::weak_ptr<SessionsManager> sessions_manager)
        : SocketBase({std::move(socket), context}), sessions_manager(std::move(sessions_manager)) {
}

ServerSideClientSession::~ServerSideClientSession() {
    spdlog::debug("ServerSideClientSession is being destroyed.");
}


void ServerSideClientSession::start() {
    socket_.handshake(boost::asio::ssl::stream_base::server);
    asyncReadMessage(MAX_FIRST_MESSAGE_SIZE, callback(&ServerSideClientSession::handleFirstRead));
}

void ServerSideClientSession::handleFirstRead(std::string_view content) {
    try {
        spdlog::debug("[ServerSideClientSession] Extracting json...");
        nlohmann::json json = InitSessionMessage::create(content);
        spdlog::debug("[ServerSideClientSession] Registering session...");
        registerSession(std::move(json));
        spdlog::debug("[ServerSideClientSession] Session registered.");
    } catch (const SessionsManagerException& e){
        std::this_thread::sleep_for(std::chrono::seconds(3)); // to prevent DDOS
        disconnect(e.what());
    } catch (const DropFileBaseException &e) {
        disconnect(e.what());
    } catch (const boost::wrapexcept<boost::system::system_error> &e) {
        spdlog::warn("Boost exception: {}", e.what());
    }
}

void ServerSideClientSession::registerSession(nlohmann::json json) {
    if (auto manager = sessions_manager.lock()) {
        if (json[InitSessionMessage::ACTION_KEY] == "send") {
            std::string session_code = manager->registerSender(std::static_pointer_cast<ServerSideClientSession>(shared_from_this()), std::move(json));
            nlohmann::json response{};
            response[InitSessionMessage::CODE_WORDS_KEY] = session_code;
            send(response.dump());
        } else {
            std::string code_words_key = json[InitSessionMessage::CODE_WORDS_KEY];
            auto [sender, session_metadata] = manager->getSenderWithMetadata(code_words_key);
            receiveFile(std::move(sender), std::move(session_metadata));
        }
    } else {
        disconnect("Internal error"); // should not ever happen
    }
}

void
ServerSideClientSession::receiveFile(std::shared_ptr<ServerSideClientSession> sender, nlohmann::json session_metadata) {
    SocketBase::send(session_metadata.dump());
    SocketBase::receiveACK();

    sender->sendACK();

    std::size_t total_received_bytes{0};
    std::size_t expected_bytes = session_metadata[InitSessionMessage::FILE_SIZE_KEY].get<std::size_t>();
    while (total_received_bytes < expected_bytes) {
        std::string data = sender->receive();
        std::size_t left_to_transfer = expected_bytes - total_received_bytes;
        std::size_t write_size = std::min(left_to_transfer, data.size());
        SocketBase::send(std::string_view{data.data(), write_size});
        SocketBase::receiveACK();

        total_received_bytes += write_size;
        sender->sendACK();
    }
}

SocketBase::MessageHandler ServerSideClientSession::callback(ServerSideClientSession::PMF pmf) {
    return [this, pmf] (std::string_view message){
        std::invoke(pmf, this, message);
    };
}
