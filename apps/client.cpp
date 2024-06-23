#include "SocketBase.hpp"
#include "InitSessionMessage.hpp"

#include "ClientSession.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <fmt/format.h>

#include <cstring>
#include <iostream>
#include <filesystem>
#include <thread>

using boost::asio::ip::tcp;



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

    ClientSession c(io_context, ctx, endpoints);

    auto t = std::jthread{[&] {
        io_context.run();
    }};

    nlohmann::json json = InitSessionMessage::create(parsed_test_file_path, action);
    c.send(json.dump());
    auto received = c.receive();
    std::cout << received;
    return 0;
}
