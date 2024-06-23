#include "DropFileServer.hpp"

#include "ServerSideClientSession.hpp"


DropFileServer::DropFileServer(unsigned short port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
          context_(boost::asio::ssl::context::sslv23) {
    context_.set_options(
            boost::asio::ssl::context::default_workarounds
            | boost::asio::ssl::context::no_sslv2
            | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
    context_.use_certificate_chain_file("/home/kuba/CLionProjects/drop-file/example_assets/cert.pem");
    context_.use_private_key_file("/home/kuba/CLionProjects/drop-file/example_assets/key.pem",
                                  boost::asio::ssl::context::pem);

    do_accept();
}

void DropFileServer::do_accept() {
    acceptor_.async_accept(
            [this](const boost::system::error_code &error, tcp::socket socket) {
                if (!error) {
                    session_manager->add(std::make_shared<ServerSideClientSession>(std::move(socket), context_));
                }

                do_accept();
            });
}

void DropFileServer::run() {
    io_service.run();
}