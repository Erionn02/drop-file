#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;

constexpr int max_length{1024};


class Client {
public:
    Client(boost::asio::io_context &io_context,
           boost::asio::ssl::context &context,
           const tcp::resolver::results_type &endpoints)
            : socket_(io_context, context) {
        socket_.set_verify_mode(boost::asio::ssl::verify_peer);
        socket_.set_verify_callback([this](bool preverified, boost::asio::ssl::verify_context &ctx){
            return verify_certificate(preverified, ctx);
        });

        connect(endpoints);
    }

private:
    bool verify_certificate(bool preverified,
                            boost::asio::ssl::verify_context &ctx) {
        char subject_name[256];
        X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, sizeof(subject_name));
        std::cout.setf(std::ios_base::boolalpha);
        std::cout << "Preverified: "<<preverified<<", Verifying " << subject_name << "\n";

        return preverified;
    }

    void connect(const tcp::resolver::results_type &endpoints) {
        socket_.lowest_layer().connect(*endpoints.begin());
        handshake();
        socket_.handshake(boost::asio::ssl::stream_base::client);
    }

    void handshake() {
        socket_.async_handshake(boost::asio::ssl::stream_base::client,
                                [this](const boost::system::error_code &error) {
                                    if (!error) {
                                        send_request();
                                    } else {
                                        std::cout << "Handshake failed: " << error.message() << "\n";
                                    }
                                });
    }

    void send_request() {
        std::cout << "Enter message: ";
        std::cin.getline(request_, max_length);
        size_t request_length = std::strlen(request_);

        boost::asio::async_write(socket_,
                                 boost::asio::buffer(request_, request_length),
                                 [this](const boost::system::error_code &error, std::size_t length) {
                                     if (!error) {
                                         receive_response(length);
                                     } else {
                                         std::cout << "Write failed: " << error.message() << "\n";
                                     }
                                 });
    }

    void receive_response(std::size_t length) {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(reply_, length),
                                [this](const boost::system::error_code &error, std::size_t length) {
                                    if (!error) {
                                        std::cout << "Reply: ";
                                        std::cout.write(reply_, (long int) length);
                                        std::cout << "\n";
                                    } else {
                                        std::cout << "Read failed: " << error.message() << "\n";
                                    }
                                });
    }

    boost::asio::ssl::stream<tcp::socket> socket_;
    char request_[max_length];
    char reply_[max_length];
};

int main() {
    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve("localhost", "12345");
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
//    ctx.set_default_verify_paths();
    ctx.load_verify_file("/home/kuba/CLionProjects/drop-file/example_assets/cert.pem");

    Client c(io_context, ctx, endpoints);

    io_context.run();


    return 0;
}