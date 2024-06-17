#include <thread>
#include <vector>
#include <spdlog/spdlog.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl.hpp>


using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket, boost::asio::ssl::context &context)
            : socket_(std::move(socket), context) {}

    void start() {
        do_handshake();
    }

private:
    void do_handshake() {
        auto self(shared_from_this());
        socket_.async_handshake(boost::asio::ssl::stream_base::server,
                                [this, self](const boost::system::error_code &error) {
                                    if (!error) {
                                        do_read();
                                    }
                                });
    }

    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_),
                                [this, self](const boost::system::error_code &ec, std::size_t length) {
                                    if (!ec) {
                                        spdlog::info("Got message, writing back...");
                                        spdlog::info("{}", std::string{data_, length});
                                        do_write(length);
                                    }
                                });
    }

    void do_write(std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                                 [this, self](const boost::system::error_code &ec,
                                              std::size_t /*length*/) {
                                     if (!ec) {
                                         do_read();
                                     }
                                 });
    }

    boost::asio::ssl::stream<tcp::socket> socket_;
    char data_[1024];
};

class Server {
public:
    Server(boost::asio::io_context &io_context, unsigned short port)
            : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
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

private:


    void do_accept() {
        acceptor_.async_accept(
                [this](const boost::system::error_code &error, tcp::socket socket) {
                    if (!error) {
                        std::make_shared<session>(std::move(socket), context_)->start();
                    }

                    do_accept();
                });
    }

    tcp::acceptor acceptor_;
    boost::asio::ssl::context context_;
};

int main() {
    boost::asio::io_service io_service;
    unsigned short port = 12345;
    spdlog::info("Starting server at port: {}", port);
    Server server(io_service, port);
    io_service.run();
}