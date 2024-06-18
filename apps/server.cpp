#include <thread>
#include <vector>
#include <spdlog/spdlog.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl.hpp>


using boost::asio::ip::tcp;

class SessionSocket {
public:
    SessionSocket(tcp::socket socket, boost::asio::ssl::context &context)
            : socket_(std::move(socket), context) {}

    void start() {
        socket_.handshake(boost::asio::ssl::stream_base::server);
    }

    std::string read(std::size_t max_read_length) {
        std::string str;
        str.resize(max_read_length);
        auto actual_read = socket_.read_some(boost::asio::buffer(str));
        str.resize(actual_read);
        return str;
    }
    void write(const std::string& message) {
        socket_.write_some(boost::asio::buffer(message));
    }

private:
    boost::asio::ssl::stream<tcp::socket> socket_;
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
                        std::make_shared<SessionSocket>(std::move(socket), context_)->start();
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