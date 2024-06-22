#include "SocketBase.hpp"
#include "InitSessionMessage.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <fmt/format.h>

#include <cstring>
#include <iostream>
#include <filesystem>
#include <thread>

using boost::asio::ip::tcp;



class Client: public SocketBase {
public:
    Client(boost::asio::io_context &io_context,
           boost::asio::ssl::context &context,
           const tcp::resolver::results_type &endpoints)
            : SocketBase({io_context, context}) {
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
        socket_.handshake(boost::asio::ssl::stream_base::client);
    }
};


int main(int argc, char* argv[]) {
    (void)argv;
    std::string parsed_test_file_path {"/home/kuba/send_test_file.txt"};
    std::string action{"receive"};
    if (argc == 1) {
        action = "send";
    }


    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve("localhost", "12345");
    assert(!endpoints.empty());
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
    ctx.load_verify_file("/home/kuba/CLionProjects/drop-file/example_assets/cert.pem");

    Client c(io_context, ctx, endpoints);

    auto t = std::jthread{[&] {
        io_context.run();
    }};

    nlohmann::json json = InitSessionMessage::create(parsed_test_file_path, action);
    c.send("wungewgegwegweg");
    auto received = c.receive();
    std::cout << received;
    return 0;
}
