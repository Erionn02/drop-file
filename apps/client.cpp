#include "InitSessionMessage.hpp"
#include "ClientSession.hpp"

#include <spdlog/spdlog.h>

#include <iostream>
#include <fstream>


int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);

    (void) argv;
    std::string parsed_test_file_path{"/home/kuba/send_test_file.txt"};

    ClientSession client("localhost", 12345, "/home/kuba/CLionProjects/drop-file/example_assets/cert.pem");
    client.start();
    spdlog::debug("Client started.");
    if (argc == 1) {
        nlohmann::json message_json = InitSessionMessage::createSendMessage(parsed_test_file_path);
        client.SocketBase::send(message_json.dump());
        spdlog::debug("Message sent.");
        std::string received_msg = client.SocketBase::receive();
        spdlog::debug("Server response: {}", received_msg);
        spdlog::info("Waiting for client to confirm");
        received_msg = client.SocketBase::receive();
        if (received_msg == "ok") {
            std::ifstream file{message_json[InitSessionMessage::FILENAME_KEY].get<std::string>()};
            client.send(std::move(file));
        }
    } else {
        nlohmann::json message_json = InitSessionMessage::createReceiveMessage(argv[1]);
        client.SocketBase::send(message_json.dump());
        spdlog::debug("Message sent.");
        auto received = client.SocketBase::receive();
        spdlog::info("Server response: {}", received);

        spdlog::info("Do you want to proceed? [y/n]");
        char confirmation{};
        std::cin >> confirmation;
        if (confirmation != 'y') {
            spdlog::info("Entered {} aborting.", confirmation);
            return 1;
        }
        auto json = nlohmann::json::parse(received);
        auto filename = json[InitSessionMessage::FILENAME_KEY].get<std::string>();
        std::ofstream received_file{filename, std::ios::trunc};
    }





    return 0;
}
