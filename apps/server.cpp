#include "SocketBase.hpp"
#include "InitSessionMessage.hpp"

#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>

#include <thread>
#include <vector>
#include <memory>
#include <iostream>

using namespace std::placeholders;

namespace asio = boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;


class ServerSessionSocket: public SocketBase, public std::enable_shared_from_this<ServerSessionSocket> {
public:
    ServerSessionSocket(tcp::socket socket, boost::asio::ssl::context &context)
            : SocketBase({std::move(socket), context}) {
    }

    template <typename PMF> auto callback(PMF pmf) { return std::bind(pmf, shared_from_this(), _1, _2); }

    void start() {
        socket_.handshake(boost::asio::ssl::stream_base::server);
        socket_.async_read_some(asio::buffer(data_), callback(&ServerSessionSocket::handle_first_read));
    }

    void disconnect(std::optional<std::string> disconnect_msg) {
        if(disconnect_msg.has_value()) {
            socket_.async_write_some(asio::buffer(*disconnect_msg), callback(&ServerSessionSocket::disconnect_send));
        } else {
            socket_.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        }
    }
private:
    void disconnect_send(error_code, size_t) {
        socket_.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_send);
    }

    void handle_first_read(error_code error, size_t bytes_transferred) {
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
                socket_.async_write_some(asio::buffer(std::string("Gotcha json buddy")), callback(&ServerSessionSocket::handle_fist_write));
                socket_.async_read_some(asio::buffer(data_), callback(&ServerSessionSocket::handle_read));
            } catch (const InitSessionMessageException& e) {
                disconnect(e.what());
            }

        }
    }

    void handle_fist_write(error_code, size_t) {

    }

    void handle_read(error_code error, size_t bytes_transferred) {
        if (!error) {
            std::cout << std::string_view(data_.data(), bytes_transferred) << std::endl;
            socket_.async_write_some(asio::buffer(std::string("ok")), callback(&ServerSessionSocket::handle_write));
        }
    }

    void handle_write(error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
            std::cout << std::string_view(data_.data(), bytes_transferred) << std::endl;
            socket_.async_read_some(asio::buffer(data_), callback(&ServerSessionSocket::handle_read));
        }
    }

//    std::array<char, sizeof(MSG_HEADER_t)> header;
};

class SessionsManager {
public:
    void add(std::shared_ptr<ServerSessionSocket> session) {
        session->start();
    }

    std::atomic_size_t connections{0};
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
                        session_manager.add(std::make_shared<ServerSessionSocket>(std::move(socket), context_));
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
    boost::asio::io_service service{10};
    unsigned short port = 12345;
    spdlog::info("Starting server at port: {}", port);
    Server server{port};
    server.run();
}