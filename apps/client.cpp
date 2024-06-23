#include "InitSessionMessage.hpp"
#include "ClientSession.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <iostream>

using boost::asio::ip::tcp;



int main(int argc, char* argv[]) {
    (void)argv;
    std::string parsed_test_file_path {"/home/kuba/send_test_file.txt"};
    std::string action{"receive"};
    if (argc == 1) {
        action = "send";
    }

    ClientSession client("localhost", 12345, "/home/kuba/CLionProjects/drop-file/example_assets/cert.pem");
    client.start();

    nlohmann::json json = InitSessionMessage::create(parsed_test_file_path, action);
    client.send(json.dump());

    auto received = client.receive();
    spdlog::info(received);

    return 0;
}
