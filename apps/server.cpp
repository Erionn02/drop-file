#include "SocketBase.hpp"

#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl.hpp>

#include <thread>
#include <vector>

using boost::asio::ip::tcp;

class SessionSocket: public SocketBase {
public:
    SessionSocket(tcp::socket socket, boost::asio::ssl::context &context)
            : SocketBase({std::move(socket), context}) {
        start();
    }

    void start() {
        socket_.handshake(boost::asio::ssl::stream_base::server);
    }
};

class SessionsManager {
public:
    void add(SessionSocket){}
};


class Server {
public:
    Server(unsigned short port)
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

    void run() {
        io_service.run();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
                [this](const boost::system::error_code &error, tcp::socket socket) {
                    if (!error) {
                        session_manager.add(SessionSocket(std::move(socket), context_));
                    }

                    do_accept();
                });
    }

    boost::asio::io_service io_service;
    tcp::acceptor acceptor_;
    boost::asio::ssl::context context_;
    SessionsManager session_manager{};
};

int main() {
    unsigned short port = 12345;
    spdlog::info("Starting server at port: {}", port);
    Server server{port};
    server.run();

}