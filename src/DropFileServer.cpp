#include "DropFileServer.hpp"
#include "ServerSideClientSession.hpp"

#include <spdlog/spdlog.h>


DropFileServer::DropFileServer(unsigned short port, const std::filesystem::path &key_cert_dir)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          context_(boost::asio::ssl::context::sslv23) {
    context_.set_options(
            boost::asio::ssl::context::default_workarounds
            | boost::asio::ssl::context::no_sslv2
            | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
    context_.use_certificate_chain_file(key_cert_dir / "cert.pem");
    context_.use_private_key_file(key_cert_dir / "key.pem",
                                  boost::asio::ssl::context::pem);

    acceptNewConnection();
}

void DropFileServer::acceptNewConnection() {
    acceptor_.async_accept(
            [this](const boost::system::error_code &error, tcp::socket socket) {
                if (!error) {
                    spdlog::info("Got new connection...");
                    try {
                        std::make_shared<ServerSideClientSession>(std::move(socket), context_,
                                                                  std::weak_ptr{session_manager})->start();
                    } catch(const std::exception& e) {
                        spdlog::error("Encountered an unexpected exception while accepting new connection: {}", e.what());
                    }
                }

                acceptNewConnection();
            });
}

void DropFileServer::run() {
    io_context.run();
}
